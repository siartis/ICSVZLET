#include <QtCore>

namespace TB {
    int I(const char f,const char M,const char I) {
        return f<<16 | M<<8 | I;
    }
    namespace METRIKA {
        QHash <QString, int> Code;
        QHash <int, QString> Name;
        QHash <int, int> Group;
    }
    namespace HELP {
        QHash <int, QString> Metrika;
        QHash <int, QString> Item;
    }
    namespace SQL {
        QHash <int, QString> Source;
        QHash <int, QString> Item;
        QHash <int, QString> Filter;
//        QHash <int, QString> Right;
    }

}

