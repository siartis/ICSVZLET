#ifndef TB_H
#define TB_H

#include <QtCore>

namespace TB {
    int I(const char f,const char M,const char I);
    namespace METRIKA {
        extern QHash <QString, int> Code;
        extern QHash <int, QString> Name;
        extern QHash <int, int> Group;
    }
    namespace SQL {
        extern QHash <int, QString> Source;
        extern QHash <int, QString> Item;
        extern QHash <int, QString> Filter;
    }
    namespace HELP {
        extern QHash <int, QString> Metrika;
        extern QHash <int, QString> Item;
    }
}

#endif // TB_H
