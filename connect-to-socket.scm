;; GCB 29mar12
;;
;; Simple test program that sends an HTTP request to a listening UNIX socket.

(use-modules (ice-9 rdelim))

(define server-socket-name
  (or (false-if-exception (cadr (command-line)))
      "/tmp/taubatron.sock"))

(define server-socket (socket PF_UNIX SOCK_STREAM 0))

(connect server-socket AF_UNIX server-socket-name)

(write-line "GET / HTTP/1.0" server-socket)
(write-line "Host: localhost" server-socket)
(newline server-socket)

(do ((next (read-line server-socket) (read-line server-socket)))((eof-object? next))
  (write-line next))

