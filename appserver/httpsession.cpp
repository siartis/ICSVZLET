#include <QtCore>
#include <QMutex>
#include <QUuid>

#include <time.h>

#include "option.h"
#include "cookie.h"
#include "../include/parser.h"
#include <QRegExp>

namespace AUTHORIZATION {
    extern COOKIE Empty;
}

namespace HTTPSESSION {
    QHash <QString, COOKIE> cookie;
    QHash <QString, QHash<QString, QString> > envSession;
    QHash <QString, QString > envAll;
    QHash <QString, QSet <QString> > envParent;
    QHash <QString, int> lastCode;
    QMutex cookieMutex;
    QMutex envMutex;
    int next_session=10;
    //QRegExp r=QRegExp(QString("%1=(.*)==").arg(OPTION::UID));

    QString getCookie(COOKIE *cookie, const QString &ses) {
        cookieMutex.lock();
        //cookie->Clear();
        *cookie=AUTHORIZATION::Empty;
        QRegExp r(QString("%1=(.*)==").arg(OPTION::UID));
        r.setMinimal(true);
        r.indexIn(ses);
        QStringList list = r.capturedTexts();
        for(int i=0;i<list.size();i++) { ///// от незнания, переделать регулярку
            list[i].chop(2);
            list[i].remove(0,OPTION::UID.length()+1);
        }
        QString new_cookie;
        foreach(QString s,list) {
            if(!HTTPSESSION::cookie.contains(s))continue;
            if(HTTPSESSION::cookie.value(s).Name!=s)continue;
            if(HTTPSESSION::cookie.value(s).LastTime>=cookie->LastTime)
                                        *cookie=HTTPSESSION::cookie.value(s);
        }
        if(cookie->Name.isEmpty()) {
            //ulong _u=(ulong)&next_session;
            //next_session=(next_session==99)?11:next_session+1;
            //new_cookie="p"+QString::number(qApp->applicationPid())+"_"+QString::number(_u);
            //for(int i=1;i<=7;i++)new_cookie.append('A'+(rand()%25));
            //new_cookie.append(QString::number(next_session));
            //while(HTTPSESSION::cookie.contains(new_cookie=QGuid))
            while(HTTPSESSION::cookie.contains(new_cookie=QUuid::createUuid().toString()));
            cookie->Name=new_cookie;
            HTTPSESSION::cookie.insert(cookie->Name,*cookie);
            QHash <QString, QString> x;
            envSession.insert(cookie->Name,x);
            lastCode.insert(cookie->Name,0);
            QSet <QString> y;
            envParent.insert(cookie->Name,y);
        }
        cookie->LastTime=time(NULL);
        HTTPSESSION::cookie.insert(cookie->Name,*cookie);
        cookieMutex.unlock();
        return(new_cookie);
    }

    void updateCookie(const COOKIE& cookie) {
        cookieMutex.lock();
        HTTPSESSION::cookie.insert(cookie.Name,cookie);
        cookieMutex.unlock();
    }

    int _env(QByteArray *anser, const QString& Path,
             const QString& param, const COOKIE& cookie) {
        bool json=Path.startsWith("json.");
        QString path=(json)?Path.mid(5):Path;
        bool all=!Path.startsWith("env.all.");
        if(!all)path.remove(3,4);
        anser->clear();
        if(path=="env.list") {
            QStringList key=(all)?envSession.value(cookie.Name).keys()
                                 :envAll.keys();
            if(json)anser->append("([");
            foreach(QString s,key)
                if(json)anser->append('\'').append(s).append("\',");
                else anser->append(s).append("\r\n");
            anser->chop((json)?(anser->endsWith(','))?1:0:2);
            if(json)anser->append("])");
            return 200;
        }
        if(path=="env.get") {
            QHash <QString, QString> x=(all)?envSession.value(cookie.Name)
                                            :envAll;
            PARSER parser(param);
            parser.Index("&");
            if(json)anser->append("({");
            for(int i=1;i<=parser.ItemCount();i++)
                if(!json)anser->append(x.value(parser.SimpleItem(i)))
                               .append("\r\n");
                else if(x.contains(parser.SimpleItem(i)))
                    anser->append(parser.SimpleItem(i)).append(":\'")
                        .append(x.value(parser.SimpleItem(i))).append("\',");
                else anser->append(parser.SimpleItem(i)).append(":NaN,");
            anser->chop((json)?(anser->endsWith(','))?1:0:2);
            if(json)anser->append("})");
            return 200;
        }
        if(path=="env.put") {
            PARSER parser(param);
            parser.Index("&");
            QString s;
            cookieMutex.lock();
            QHash <QString, QString> *x=(all)?&envSession[cookie.Name]:&envAll;
            QSet <QString> *parent=(all)?NULL:&envParent[cookie.Name];
            for(int j,i=1;i<=parser.ItemCount();i++) {
                s=parser.SimpleItem(i);
                j=s.indexOf("=");
                if(j==-1)continue;
                if(parent!=NULL)
                    if(x->contains(s.left(j)))
                        if(!parent->contains(s.left(j)))continue;
                if(parent!=NULL)parent->insert(s.left(j));
                anser->append(s.left(j)).append("\r\n");
                x->insert(s.left(j),s.mid(j+1));
            }
            cookieMutex.unlock();
            return 200;
        }
        if(path=="env.erase") {
            PARSER parser(param);
            parser.Index("&");
            cookieMutex.lock();
            QHash <QString, QString> *x=(all)?&envSession[cookie.Name]:&envAll;
            QSet <QString> *parent=(all)?NULL:&envParent[cookie.Name];
            if(json)anser->append("([");
            for(int i=1;i<=parser.ItemCount();i++) {
                if(!x->contains(parser.SimpleItem(i)))continue;
                if(parent!=NULL)
                    if(!parent->contains(parser.SimpleItem(i)))continue;
                if(parent!=NULL)parent->remove(parser.SimpleItem(i));
                x->remove(parser.SimpleItem(i));
                if(json)anser->append('\'')
                              .append(parser.SimpleItem(i))
                              .append("\',");
                else anser->append(parser.SimpleItem(i)).append("\r\n");
            }
            anser->chop((json)?(anser->endsWith(','))?1:0:2);
            if(json)anser->append("])");
            cookieMutex.unlock();
            return 200;
        }
        return 404;
    }

    int env(QByteArray *anser, const QString& Path,
            const QString& param, const COOKIE& cookie) {
        envMutex.lock();
        int i=_env(anser,Path,param,cookie);
        envMutex.unlock();
        return i;
    }

}
