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
        case 'C0': return('À');
        case 'C1': return('Á');
        case 'C2': return('Â');
        case 'C3': return('Ã');
        case 'C4': return('Ä');
        case 'C5': return('Å');
        case 'A8': return('¨');
        case 'C6': return('Æ');
        case 'C7': return('Ç');
        case 'C8': return('È');
        case 'C9': return('É');
        case 'CA': return('Ê');
        case 'CB': return('Ë');
        case 'CC': return('Ì');
        case 'CD': return('Í');
        case 'CE': return('Î');
        case 'CF': return('Ï');
        case 'D0': return('Ð');
        case 'D1': return('Ñ');
        case 'D2': return('Ò');
        case 'D3': return('Ó');
        case 'D4': return('Ô');
        case 'D5': return('Õ');
        case 'D6': return('Ö');
        case 'D7': return('×');
        case 'DA': return('Ú');
        case 'DB': return('Û');
        case 'DC': return('Ü');
        case 'DD': return('Ý');
        case 'DE': return('Þ');
        case 'DF': return('ß');
        case 'E0': return('à');
        case 'E1': return('á');
        case 'E2': return('â');
        case 'E3': return('ã');
        case 'E4': return('ä');
        case 'E5': return('å');
        case 'B8': return('¸');
        case 'E6': return('æ');
        case 'E7': return('ç');
        case 'E8': return('è');
        case 'E9': return('é');
        case 'EA': return('ê');
        case 'EB': return('ë');
        case 'EC': return('ì');
        case 'ED': return('í');
        case 'EE': return('î');
        case 'EF': return('ï');
        case 'F0': return('ð');
        case 'F1': return('ñ');
        case 'F2': return('ò');
        case 'F3': return('ó');
        case 'F4': return('ô');
        case 'F5': return('õ');
        case 'F6': return('ö');
        case 'F7': return('÷');
        case 'FA': return('ú');
        case 'FB': return('û');
        case 'FC': return('ü');
        case 'FD': return('ý');
        case 'FE': return('þ');
        case 'FF': return('ÿ');
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
