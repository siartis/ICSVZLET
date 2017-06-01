#include <QtSql>
#include <QMutex>
#include <QMultiHash>
#include <QPair>
#include "../include/table.h"
#include "tb.h"
#include "cookie.h"

bool testWrite(const COOKIE& cookie, const long M, const long ID, const PARSER& p) {
    if(cookie.roleName=="Администратор")return true;
    if(TB::METRIKA::Name.value(M)=="Person")if(ID==0)return true;
    if(TB::METRIKA::Name.value(M)=="University")if(ID==cookie.personID)return true;

    return false;
}

bool testErase(const COOKIE& cookie, const long M, const long ID) {
    if(cookie.roleName=="Администратор")return true;
    return false;
}

bool testRead(const COOKIE& cookie, const long M, const QList <long>& ID,
              TABLE *anser) {
    return true;
}

namespace API {

    QMutex F;
    QSqlDatabase db;

    QVector<long> Vector(const register char *s) {
        QVector<long> v;
        if(*s!='\0')v.append(atol(s));
        for(register int i=0;s[i]!='\0';i++)
            if(s[i]==',')v.append(atol(s+(++i)));
        return v;
    }

    QVector<long> Vector(const QString& t) {
        //return Vector(t.toAscii().data());
        return Vector(t.toLocal8Bit().data());
    }

    QString String(const long n,const long *v) {
        char t[32];
        QString s;
        for(int i=0;i<n;i++)s.append(ltoa(v[i],t,10)).append(',');
        s.chop(1);
        return(s);
    }

    QString String(const QVector<long>& v) {
        return(String(v.size(),v.data()));
    }

    long READ(const QString &sql, TABLE *res) {
        // Возврат - количество прочитанных записей
        QSqlQuery query(db);
        query.setForwardOnly(true);
        if(!query.exec(sql))return -1;
        if(res==NULL)return(query.value(0).toInt());
        res->clear();
        long k=0;
        while(query.next())    {
            QSqlRecord r=query.record();
            for(int i=1;i<=r.count();i++)    {
                if(r.value(i-1).type()==QVariant::DateTime)
                    res->append(r.value(i-1).toDate().toString());
                else res->append(r.value(i-1).toString());
                res->append('\r');
            }
            res->append('\n');
            k++;
        }
        if(res->endsWith("\r\n"))res->chop(2);
        if(res->size()==0 && k>0)res->append('\0');
        if(res->Index("\r")<0)return -1;
        return(k);
    }

    long GetChild(const long M, const long ID,
                  QVector <long> *v) { // Возврат - количество детей
        //QHash <long,QVector <long> > *h=GetHash(M);
        QHash <long,QVector <long> > *h=NULL;
        v->clear();
        bool f=(h==NULL)?false:((h->contains(ID))?true:false);
        if(f)*v=h->value(ID);
        else {
            int i;
            int r=v->size();
            QString s=QString("%1").arg(ID);
            QString sql=QString("SELECT %1ID FROM %1 "
                                "WHERE (%1IDPARENT in (%2) "
                                "and %1ID>1)").arg(TB::METRIKA::Name.value(M))
                                              .arg("%1");
            QSqlQuery query(db);
            do {
                if(!query.exec(sql.arg(s)))return -1;
                while(query.next())v->append(query.value(0).toInt());
                i=(v->size()-r>100)?100:v->size()-r;
                s=String(i,v->data()+r);
                r+=i;
            }
            while(!s.isEmpty());
            if(h!=NULL)h->insert(ID,*v);
        }
        return(v->size());
    }

    long GetChild(const bool f, // f - нужно или нет возвращать сам ID
                  const long M, const long ID,
                  QString *Child) { // Возврат - количество детей
        QVector <long> v;
        int r=GetChild(M,ID,&v);
        if(r<0)return -1;
        *Child=String(v);
        if(f) {
            if(!Child->isEmpty())Child->append(',');
            char t[32];
            Child->append(ltoa(ID,t,10));
            r++;
        }
        return(r);
    }

    long ExpandID(const long M, const long ID, const long *id,
                  QString *Child) { // Возврат - количество детей
        if(TB::METRIKA::Group.value(M)!=M && ID<0)return -1;else Child->clear();
        if(ID<0)return(GetChild(true,M,-ID,Child));
        int k,r=(ID>0)?1:0;
        if(id!=NULL) {
            r=0;
            QString h;
            for(int i=1;i<=ID;i++)  {
                if((k=ExpandID(M,id[i-1],NULL,&h))<0)return -1;
                Child->append(h);
                if(i!=ID)Child->append(",");
                r+=k;
            }
        }
        else if(ID>0)*Child=QString("%1").arg(ID);
        return(r);
    }

    long Get(const char *fm, // Формат
             const long M,const QList <long>& M_Filter,
             const QString& filter,TABLE *table) {
        QString select;
        long i=0;
        char f;
        while((f=fm[i])!='\0')    {
            if(!isdigit(f))
                select.append(TB::SQL::Item.value(TB::I(true,M,f)))
                      .append(",");
            else if(f=='0')
                select.append(TB::SQL::Filter.value(TB::I(false,M,0)))
                      .append(",");
            else if(f-'0'>M_Filter.size())return -1;
            else select.append(TB::SQL::Filter.value(TB::I(false,M,
                                                     M_Filter.at(f-'1'))))
                       .append(",");
            i++;
        }
        select.chop(1);
        QString SQL=QString(TB::SQL::Source.value(TB::I(true,M,0)));
        if(filter.isEmpty())SQL.remove(" AND %2").remove("WHERE %2");
        SQL=QString(SQL).arg(select).arg(filter);
        return READ(SQL,table);
    }

    long Get(const char *fm, const long M,const QList <long>& M_Filter,
             const QMultiHash <long,long>& key, TABLE *table) {
        QString filter;
        QString child;
        QVector <long> id;
        for(int i=0;i<M_Filter.size();i++) {
            id=key.values(M_Filter.at(i)).toVector();
            if(M_Filter.at(i)==0)
                for(int j=0;j<id.size();j++)if(id.at(j)<=0)return -1;
            if(ExpandID(M_Filter.at(i),id.size(),id.data(),&child)<0)return -1;
            filter.append(QString("%1 in (%2) and ")
                          .arg(TB::SQL::Filter.value(TB::I(false,M,
                                                     M_Filter.at(i)))).arg(child));
        }
        filter.chop(5);
        return Get(fm,M,M_Filter,filter,table);
    }

    long Put(const long M,const long ID,const PARSER& parser) {
        if(ID<0)return -1;
        QString select;
        const char *q;
        for(int i=1;i<=parser.ItemCount();i++)
            if((q=parser.SimpleItem(i))[0]=='\0')return -1;
            else if(q[1]!='=')return -1;
            else select.append(TB::SQL::Item.value(TB::I(false,M,q[0]))).append(",");
        if(ID>0)select.replace(",","=?,");
        select.chop(1);
        //select.remove(QString("%1.").arg(TB::METRIKA::Name.value(M)),Qt::CaseInsensitive);
        QString s=QString("?,").repeated(parser.ItemCount());
        s.chop(1);
        QString sql=(ID>0)?QString("UPDATE %1 SET %2  WHERE %1ID=%3")
                           .arg(TB::METRIKA::Name.value(M)).arg(select).arg(ID)
                          :QString("INSERT INTO %1 (%2) VALUES (%3)")
                           .arg(TB::METRIKA::Name.value(M)).arg(select)
                           .arg(s);
        QSqlQuery query(db);
        if(!query.prepare(sql))return -1;
        for(int i=1;i<=parser.ItemCount();i++)
            query.bindValue(i-1,QVariant(parser.SimpleItem(i)+2));
        if(!query.exec())return -1;
        if(ID>0)return(ID);
        if(!query.exec(QString("SELECT Last(%1ID) FROM %1")
                       .arg(TB::METRIKA::Name.value(M))))return -1;
        if(!query.next())return -1;
        return(query.value(0).toInt());
    }

    long Erase(const long M, const long ID) { // Возврат - ID удаляемой записи
        QSqlQuery query(db);
        if(!query.exec(QString("DELETE %1ID FROM %1 WHERE %1ID=%2")
                       .arg(TB::METRIKA::Name.value(M)).arg(ID)))return -1;
        return ID;
    }

    long _API(const COOKIE& cookie,
              TABLE *anser,
              const QString &command,const QString &param) {
        QString path=command;
        int code=0;
        PARSER parser(param);
        parser.Index("&");
        if(path.endsWith("Put")) {
            if(parser.ItemCount()<2 || parser.ItemCount()>25)return -1;
            path.chop(3);
            int M=TB::METRIKA::Code.value(path);
            long ID=parser.SimpleItemInt(1);
            //if(M==-1 || ID==1 || ID<0)return -1;
            if(M==-1 || ID<0)return -1;
            parser.RemoveAt(1);
            if(!testWrite(cookie,M,ID,parser))return -1;  // проверка прав
            if(code==0 && M!=0)code=Put(M,ID,parser);else code=-1;
            anser->setNum(code);
            anser->Index(";");
        }
        else if(path.endsWith("Erase")) {
            if(parser.ItemCount()!=1)return -1;
            path.chop(5);
            int M=TB::METRIKA::Code.value(path);
            long ID=parser.SimpleItemInt(1);
            if(M==-1 || ID==1 || ID<0)return -1;
            if(!testErase(cookie,M,ID))return -1;  // проверка прав
            if(code==0 && M!=0)code=Erase(M,ID);else code=-1;
            anser->setNum(code);
            anser->Index(";");
        }
        else if(path.endsWith("Get")) {
            if(parser.ItemCount()<1 || parser.ItemCount()>7)return -1;
            path.chop(3);
            int M=TB::METRIKA::Code.value(path,-1);
            if(M==-1)return -1;
            QMultiHash <long,long> key;
            const char *x,*z;
            long M_Parent;
            QVector <long> id;
            QList <long> M_Filter;
            for(int i=2;i<=parser.ItemCount();i++) {
                z=parser.SimpleItem(i);
                x=strchr(z,'=');
                //M_Parent=(x==NULL)?0:TB::METRIKA::Code.value(QString::fromAscii(z,x-z),-1);
                M_Parent=(x==NULL)?0:TB::METRIKA::Code.value(QString::fromLocal8Bit(z,x-z),-1);
                if(M_Parent==-1)return -1;
                id=Vector((x==NULL)?z:x+1);
                for(int j=0;j<id.size();j++)key.insertMulti(M_Parent,id.at(j));
                M_Filter.append(M_Parent);
            }
            if(code==0 && M!=0)
                code=Get(parser.SimpleItem(1),M,M_Filter,key,anser);
            else code=-1;
            // проверка прав
            QList <long> fid;
            if(code>0)if(!testRead(cookie,M,fid,anser))return -1;
        }
        else code=-1;
        return code;
    }

    long API(const COOKIE& cookie,TABLE *anser,const QString &command,const QString &param) {
        F.lock();
        int code=_API(cookie,anser,command,param);
        F.unlock();
        return code;
    }

}
