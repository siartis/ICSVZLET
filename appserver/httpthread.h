#ifndef HTTPTHREAD_H
#define HTTPTHREAD_H

class HttpRequest;

namespace HTTPTHREAD {
    extern HttpRequest **ThreadPtr;
    extern int *ThreadCode;
}

#endif // HTTPTHREAD_H
