#include <QtCore>

int PageDecode(QByteArray *source, const int page) {
    switch (page) {
        //case 8:*source=QString::fromUtf8(source->data()).toAscii();
    case 8:*source=QString::fromUtf8(source->data()).toLocal8Bit();
               return 8;
    }
    return 0;
}

void UrlDecode(QByteArray *source) {
    //QString a0=source->data();
    long pos=0;
    char *in=source->data();
    const long len=source->size();
    for(long i=0;i<len;) {
        if(in[i++]!='%')in[pos++]=in[i-1];
        else if(in[i]=='%')in[pos++]='%';
        else {
            if (in[i]>=97) in[i]-=32;
            in[pos++]=char(char((in[i]-(in[i]<64?48:55))<<4)|
                           char((in[i+1]-(in[i+1]<64?48:55))));
            i+=2;
        }
    }
    in[pos]=0;
    source->resize(pos);
    return;
}

/*
char url_decode_char(short k) {
    switch(k) {
        case '21': return('!');
        case '22': return('"');
        case '23': return('#');
        case '25': return('%');
        case '26': return('&');
        case '27': return('\'');
        case '2A': return('*');
        case '2C': return(',');
        case '3A': return(':');
        case '3B': return(';');
        case '3C': return('<');
        case '3E': return('>');
        case '3F': return('?');
        case '5B': return('[');
        case '5D': return(']');
        case '5E': return('^');
        case '60': return('`');
        case '7B': return('{');
        case '7C': return('|');
        case '7D': return('}');
        case '20': return(' ');
        case 'C0': return('�');
        case 'C1': return('�');
        case 'C2': return('�');
        case 'C3': return('�');
        case 'C4': return('�');
        case 'C5': return('�');
        case 'A8': return('�');
        case 'C6': return('�');
        case 'C7': return('�');
        case 'C8': return('�');
        case 'C9': return('�');
        case 'CA': return('�');
        case 'CB': return('�');
        case 'CC': return('�');
        case 'CD': return('�');
        case 'CE': return('�');
        case 'CF': return('�');
        case 'D0': return('�');
        case 'D1': return('�');
        case 'D2': return('�');
        case 'D3': return('�');
        case 'D4': return('�');
        case 'D5': return('�');
        case 'D6': return('�');
        case 'D7': return('�');
        case 'DA': return('�');
        case 'DB': return('�');
        case 'DC': return('�');
        case 'DD': return('�');
        case 'DE': return('�');
        case 'DF': return('�');
        case 'E0': return('�');
        case 'E1': return('�');
        case 'E2': return('�');
        case 'E3': return('�');
        case 'E4': return('�');
        case 'E5': return('�');
        case 'B8': return('�');
        case 'E6': return('�');
        case 'E7': return('�');
        case 'E8': return('�');
        case 'E9': return('�');
        case 'EA': return('�');
        case 'EB': return('�');
        case 'EC': return('�');
        case 'ED': return('�');
        case 'EE': return('�');
        case 'EF': return('�');
        case 'F0': return('�');
        case 'F1': return('�');
        case 'F2': return('�');
        case 'F3': return('�');
        case 'F4': return('�');
        case 'F5': return('�');
        case 'F6': return('�');
        case 'F7': return('�');
        case 'FA': return('�');
        case 'FB': return('�');
        case 'FC': return('�');
        case 'FD': return('�');
        case 'FE': return('�');
        case 'FF': return('�');
    }
    return('_');
}

void UrlDecode(QByteArray *in) {

    //QString a1=QByteArray::fromPercentEncoding(*in);
    //return;
    int pos=0;
    char *p=in->data();
    for(int i=0;i<in->size();i++)
        if (p[i]!='%')p[pos++]=p[i];
        else if(p[++i]=='%')p[pos++]='%';
        else p[pos++]=url_decode_char((short(p[++i-1])<<8)+short(p[i]));
    p[pos++]='\0';
    QString a=p;
    a="";
}
*/
