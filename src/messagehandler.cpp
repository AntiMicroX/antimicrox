#include "messagehandler.h"

#include <stdio.h>
#include <stdlib.h>

namespace MessageHandler
{
   void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
   {

       fprintf(stderr, "Debug: %s (%s:%u, %s)\n", msg.toLocal8Bit().constData(), context.file, context.line, context.function);
    #ifndef QT_DEBUG_NO_OUTPUT

       QByteArray localMsg = msg.toLocal8Bit();
       switch (type) {
       case QtDebugMsg:
           fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
           break;
       case QtInfoMsg:
           fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
           break;
       case QtWarningMsg:
           fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
           break;
       case QtCriticalMsg:
           fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
           break;
       case QtFatalMsg:
           fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
           abort();
       }

    #endif
   }
}


