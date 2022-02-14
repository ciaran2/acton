#pragma once
//////////////////////////////////////////////////////////////////////////////////
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <strings.h>
#include <unistd.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include "builtin.h"
#include "../rts/rts.h"

#ifdef IS_MACOS
#include <sys/event.h>
typedef struct kevent EVENT_type;
#endif

#ifdef IS_GNU_LINUX
#include <sys/epoll.h>
typedef struct epoll_event EVENT_type;
#endif

#define BUF_SIZE 1024

#define MAX_FD  100

typedef enum HandlerCase {nohandler, readhandler, connecthandler} HandlerCase;


///////////////////////////////////////////////////////////////////////////////////////////
// START GENERATED __builtin__.act
struct $l$1lambda;
struct $l$2lambda;
struct $l$3lambda;
struct $l$4lambda;
struct $l$5lambda;
struct $l$6lambda;
struct $l$7lambda;
struct $l$8lambda;
struct $l$9lambda;
struct $l$10lambda;
struct $l$11lambda;
struct $l$12lambda;
struct $l$13lambda;
struct $l$14lambda;
struct $Env;
struct $Connection;
struct $RFile;
struct $WFile;
typedef struct $l$1lambda *$l$1lambda;
typedef struct $l$2lambda *$l$2lambda;
typedef struct $l$3lambda *$l$3lambda;
typedef struct $l$4lambda *$l$4lambda;
typedef struct $l$5lambda *$l$5lambda;
typedef struct $l$6lambda *$l$6lambda;
typedef struct $l$7lambda *$l$7lambda;
typedef struct $l$8lambda *$l$8lambda;
typedef struct $l$9lambda *$l$9lambda;
typedef struct $l$10lambda *$l$10lambda;
typedef struct $l$11lambda *$l$11lambda;
typedef struct $l$12lambda *$l$12lambda;
typedef struct $l$13lambda *$l$13lambda;
typedef struct $l$14lambda *$l$14lambda;
typedef struct $Env *$Env;
typedef struct $Connection *$Connection;
typedef struct $RFile *$RFile;
typedef struct $WFile *$WFile;
struct $l$1lambda$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($l$1lambda, $Env, $str);
    void (*__serialize__) ($l$1lambda, $Serial$state);
    $l$1lambda (*__deserialize__) ($l$1lambda, $Serial$state);
    $bool (*__bool__) ($l$1lambda);
    $str (*__str__) ($l$1lambda);
    $R (*__call__) ($l$1lambda, $Cont);
};
struct $l$1lambda {
    struct $l$1lambda$class *$class;
    $Env __self__;
    $str s;
};
struct $l$2lambda$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($l$2lambda, $Env, $function);
    void (*__serialize__) ($l$2lambda, $Serial$state);
    $l$2lambda (*__deserialize__) ($l$2lambda, $Serial$state);
    $bool (*__bool__) ($l$2lambda);
    $str (*__str__) ($l$2lambda);
    $R (*__call__) ($l$2lambda, $Cont);
};
struct $l$2lambda {
    struct $l$2lambda$class *$class;
    $Env __self__;
    $function cb;
};
struct $l$3lambda$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($l$3lambda, $Env, $str, $int, $function);
    void (*__serialize__) ($l$3lambda, $Serial$state);
    $l$3lambda (*__deserialize__) ($l$3lambda, $Serial$state);
    $bool (*__bool__) ($l$3lambda);
    $str (*__str__) ($l$3lambda);
    $R (*__call__) ($l$3lambda, $Cont);
};
struct $l$3lambda {
    struct $l$3lambda$class *$class;
    $Env __self__;
    $str host;
    $int port;
    $function cb;
};
struct $l$4lambda$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($l$4lambda, $Env, $int, $function, $function);
    void (*__serialize__) ($l$4lambda, $Serial$state);
    $l$4lambda (*__deserialize__) ($l$4lambda, $Serial$state);
    $bool (*__bool__) ($l$4lambda);
    $str (*__str__) ($l$4lambda);
    $R (*__call__) ($l$4lambda, $Cont);
};
struct $l$4lambda {
    struct $l$4lambda$class *$class;
    $Env __self__;
    $int port;
    $function cb_on_connect;
    $function cb_on_error;
};
struct $l$5lambda$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($l$5lambda, $Env, $int);
    void (*__serialize__) ($l$5lambda, $Serial$state);
    $l$5lambda (*__deserialize__) ($l$5lambda, $Serial$state);
    $bool (*__bool__) ($l$5lambda);
    $str (*__str__) ($l$5lambda);
    $R (*__call__) ($l$5lambda, $Cont);
};
struct $l$5lambda {
    struct $l$5lambda$class *$class;
    $Env __self__;
    $int n;
};
struct $l$6lambda$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($l$6lambda, $Env, $str);
    void (*__serialize__) ($l$6lambda, $Serial$state);
    $l$6lambda (*__deserialize__) ($l$6lambda, $Serial$state);
    $bool (*__bool__) ($l$6lambda);
    $str (*__str__) ($l$6lambda);
    $R (*__call__) ($l$6lambda, $Cont);
};
struct $l$6lambda {
    struct $l$6lambda$class *$class;
    $Env __self__;
    $str nm;
};
struct $l$7lambda$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($l$7lambda, $Env, $str);
    void (*__serialize__) ($l$7lambda, $Serial$state);
    $l$7lambda (*__deserialize__) ($l$7lambda, $Serial$state);
    $bool (*__bool__) ($l$7lambda);
    $str (*__str__) ($l$7lambda);
    $R (*__call__) ($l$7lambda, $Cont);
};
struct $l$7lambda {
    struct $l$7lambda$class *$class;
    $Env __self__;
    $str nm;
};
struct $l$8lambda$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($l$8lambda, $Connection, $str);
    void (*__serialize__) ($l$8lambda, $Serial$state);
    $l$8lambda (*__deserialize__) ($l$8lambda, $Serial$state);
    $bool (*__bool__) ($l$8lambda);
    $str (*__str__) ($l$8lambda);
    $R (*__call__) ($l$8lambda, $Cont);
};
struct $l$8lambda {
    struct $l$8lambda$class *$class;
    $Connection __self__;
    $str s;
};
struct $l$9lambda$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($l$9lambda, $Connection);
    void (*__serialize__) ($l$9lambda, $Serial$state);
    $l$9lambda (*__deserialize__) ($l$9lambda, $Serial$state);
    $bool (*__bool__) ($l$9lambda);
    $str (*__str__) ($l$9lambda);
    $R (*__call__) ($l$9lambda, $Cont);
};
struct $l$9lambda {
    struct $l$9lambda$class *$class;
    $Connection __self__;
};
struct $l$10lambda$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($l$10lambda, $Connection, $function, $function);
    void (*__serialize__) ($l$10lambda, $Serial$state);
    $l$10lambda (*__deserialize__) ($l$10lambda, $Serial$state);
    $bool (*__bool__) ($l$10lambda);
    $str (*__str__) ($l$10lambda);
    $R (*__call__) ($l$10lambda, $Cont);
};
struct $l$10lambda {
    struct $l$10lambda$class *$class;
    $Connection __self__;
    $function cb1;
    $function cb2;
};
struct $l$11lambda$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($l$11lambda, $RFile);
    void (*__serialize__) ($l$11lambda, $Serial$state);
    $l$11lambda (*__deserialize__) ($l$11lambda, $Serial$state);
    $bool (*__bool__) ($l$11lambda);
    $str (*__str__) ($l$11lambda);
    $R (*__call__) ($l$11lambda, $Cont);
};
struct $l$11lambda {
    struct $l$11lambda$class *$class;
    $RFile __self__;
};
struct $l$12lambda$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($l$12lambda, $RFile);
    void (*__serialize__) ($l$12lambda, $Serial$state);
    $l$12lambda (*__deserialize__) ($l$12lambda, $Serial$state);
    $bool (*__bool__) ($l$12lambda);
    $str (*__str__) ($l$12lambda);
    $R (*__call__) ($l$12lambda, $Cont);
};
struct $l$12lambda {
    struct $l$12lambda$class *$class;
    $RFile __self__;
};
struct $l$13lambda$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($l$13lambda, $WFile, $str);
    void (*__serialize__) ($l$13lambda, $Serial$state);
    $l$13lambda (*__deserialize__) ($l$13lambda, $Serial$state);
    $bool (*__bool__) ($l$13lambda);
    $str (*__str__) ($l$13lambda);
    $R (*__call__) ($l$13lambda, $Cont);
};
struct $l$13lambda {
    struct $l$13lambda$class *$class;
    $WFile __self__;
    $str s;
};
struct $l$14lambda$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($l$14lambda, $WFile);
    void (*__serialize__) ($l$14lambda, $Serial$state);
    $l$14lambda (*__deserialize__) ($l$14lambda, $Serial$state);
    $bool (*__bool__) ($l$14lambda);
    $str (*__str__) ($l$14lambda);
    $R (*__call__) ($l$14lambda, $Cont);
};
struct $l$14lambda {
    struct $l$14lambda$class *$class;
    $WFile __self__;
};
extern struct $l$1lambda$class $l$1lambda$methods;
$l$1lambda $l$1lambda$new($Env, $str);
extern struct $l$2lambda$class $l$2lambda$methods;
$l$2lambda $l$2lambda$new($Env, $function);
extern struct $l$3lambda$class $l$3lambda$methods;
$l$3lambda $l$3lambda$new($Env, $str, $int, $function);
extern struct $l$4lambda$class $l$4lambda$methods;
$l$4lambda $l$4lambda$new($Env, $int, $function, $function);
extern struct $l$5lambda$class $l$5lambda$methods;
$l$5lambda $l$5lambda$new($Env, $int);
extern struct $l$6lambda$class $l$6lambda$methods;
$l$6lambda $l$6lambda$new($Env, $str);
extern struct $l$7lambda$class $l$7lambda$methods;
$l$7lambda $l$7lambda$new($Env, $str);
extern struct $l$8lambda$class $l$8lambda$methods;
$l$8lambda $l$8lambda$new($Connection, $str);
extern struct $l$9lambda$class $l$9lambda$methods;
$l$9lambda $l$9lambda$new($Connection);
extern struct $l$10lambda$class $l$10lambda$methods;
$l$10lambda $l$10lambda$new($Connection, $function, $function);
extern struct $l$11lambda$class $l$11lambda$methods;
$l$11lambda $l$11lambda$new($RFile);
extern struct $l$12lambda$class $l$12lambda$methods;
$l$12lambda $l$12lambda$new($RFile);
extern struct $l$13lambda$class $l$13lambda$methods;
$l$13lambda $l$13lambda$new($WFile, $str);
extern struct $l$14lambda$class $l$14lambda$methods;
$l$14lambda $l$14lambda$new($WFile);
// END GENERATED __builtin__.act
///////////////////////////////////////////////////////////////////////////////////////////

struct $Env$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($Env, $list);
    void (*__serialize__) ($Env, $Serial$state);
    $Env (*__deserialize__) ($Env, $Serial$state);
    $bool (*__bool__) ($Env);
    $str (*__str__) ($Env);
    $R (*stdout_write$local) ($Env, $str, $Cont);
    $R (*stdin_install$local) ($Env, $function, $Cont);
    $R (*connect$local) ($Env, $str, $int, $function, $Cont);
    $R (*listen$local) ($Env, $int, $function, $function, $Cont);
    $R (*exit$local) ($Env, $int, $Cont);
    $R (*openR$local) ($Env, $str, $Cont);
    $R (*openW$local) ($Env, $str, $Cont);
    $Msg (*stdout_write) ($Env, $str);
    $Msg (*stdin_install) ($Env, $function);
    $Msg (*connect) ($Env, $str, $int, $function);
    $Msg (*listen) ($Env, $int, $function, $function);
    $Msg (*exit) ($Env, $int);
    $Msg (*openR) ($Env, $str);
    $Msg (*openW) ($Env, $str);
};
struct $Env {
    struct $Env$class *$class;
    $Actor $next;
    $Msg $msg;
    $Msg $outgoing;
    $Msg $waitsfor;
    $int64 $consume_hd;
    $Catcher $catcher;
    $Lock $msg_lock;
    $long $globkey;
    $list argv;
};
struct $Connection$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($Connection, int);
    void (*__serialize__) ($Connection, $Serial$state);
    $Connection (*__deserialize__) ($Connection, $Serial$state);
    $bool (*__bool__) ($Connection);
    $str (*__str__) ($Connection);
    $R (*write$local) ($Connection, $str, $Cont);
    $R (*close$local) ($Connection, $Cont);
    $R (*on_receipt$local) ($Connection, $function, $function, $Cont);
    $Msg (*write) ($Connection, $str);
    $Msg (*close) ($Connection);
    $Msg (*on_receipt) ($Connection, $function, $function);
};
struct $Connection {
    struct $Connection$class *$class;
    $Actor $next;
    $Msg $msg;
    $Msg $outgoing;
    $Msg $waitsfor;
    $int64 $consume_hd;
    $Catcher $catcher;
    $Lock $msg_lock;
    $long $globkey;
    int descriptor;
};
struct $RFile$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($RFile, FILE*);
    void (*__serialize__) ($RFile, $Serial$state);
    $RFile (*__deserialize__) ($RFile, $Serial$state);
    $bool (*__bool__) ($RFile);
    $str (*__str__) ($RFile);
    $R (*readln$local) ($RFile, $Cont);
    $R (*close$local) ($RFile, $Cont);
    $Msg (*readln) ($RFile);
    $Msg (*close) ($RFile);
};
struct $RFile {
    struct $RFile$class *$class;
    $Actor $next;
    $Msg $msg;
    $Msg $outgoing;
    $Msg $waitsfor;
    $int64 $consume_hd;
    $Catcher $catcher;
    $Lock $msg_lock;
    $long $globkey;
    FILE *file;
};
struct $WFile$class {
    char *$GCINFO;
    int $class_id;
    $Super$class $superclass;
    $NoneType (*__init__) ($WFile, int);
    void (*__serialize__) ($WFile, $Serial$state);
    $WFile (*__deserialize__) ($WFile, $Serial$state);
    $bool (*__bool__) ($WFile);
    $str (*__str__) ($WFile);
    $R (*write$local) ($WFile, $str, $Cont);
    $R (*close$local) ($WFile, $Cont);
    $Msg (*write) ($WFile, $str);
    $Msg (*close) ($WFile);
};
struct $WFile {
    struct $WFile$class *$class;
    $Actor $next;
    $Msg $msg;
    $Msg $outgoing;
    $Msg $waitsfor;
    $int64 $consume_hd;
    $Catcher $catcher;
    $Lock $msg_lock;
    $long $globkey;
    int descriptor;
};
extern struct $Env$class $Env$methods;
$Env $Env$newact($list);
extern struct $Connection$class $Connection$methods;
$Connection $Connection$newact(int);
extern struct $RFile$class $RFile$methods;
$RFile $RFile$newact(FILE*);
extern struct $WFile$class $WFile$methods;
$WFile $WFile$newact(int);
void $__init__();

struct FileDescriptorData {
  HandlerCase kind;
  $function rhandler;
  $function errhandler;
  $function chandler;
  $Connection conn;
  struct sockaddr_in sock_addr;
  EVENT_type event_spec;
  char buffer[BUF_SIZE];
  int bufnxt;              // only used for RFiles; index of first unreported char
  int bufused;             //        -"-          ; nr of read chars in buffer. Equal to BUF_SIZE except before first read and (possibly) after last read.
};

extern struct FileDescriptorData fd_data[MAX_FD];
extern int kq;

void reset_timeout();

void setupConnection (int fd);
$str $getName(int fd);
void *$eventloop(void *);
