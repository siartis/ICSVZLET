#include <QtCore>

bool HttpSplint(QUrl *url, QHash <QString,QString> *env,
                QByteArray *param, const QByteArray& in) {
    if(param!=NULL)if(url==NULL)return false;
    if(url!=NULL)url->clear();
    if(env!=NULL)env->clear();
    if(param!=NULL)param->clear();
    int k=in.indexOf("\r\n\r\n");
    if(k==-1)return false;
    int j=(in.startsWith("GET /"))?5:((in.startsWith("POST /"))?6:0);
    if(j==0) return false;
    int i=in.indexOf('\r');
    //if(url!=NULL)url->setUrl(QString::fromAscii(in.data()+j,i-9-j));
    if(url!=NULL)url->setUrl(QString::fromLocal8Bit(in.data()+j,i-9-j));
    if(env!=NULL) {
        //QStringList x=(QString::fromAscii(in.data()+i+2,k-i-2)).split("\r\n");
        QStringList x=(QString::fromLocal8Bit(in.data()+i+2,k-i-2)).split("\r\n");
        for(i=0;i<x.size();i++) {
            if((k=x.at(i).indexOf(": "))==-1)return false;
            env->insert(x.at(i).left(k),x.at(i).mid(k+1));
        }
    }
    //if(param!=NULL)*param=url->toString().mid(url->path().length()+1).toAscii();
    if(param!=NULL)*param=url->toString().mid(url->path().length()+1).toLocal8Bit();
    return true;
}

