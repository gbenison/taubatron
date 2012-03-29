(use-modules (web server))

(define (dumb-handler . args)
  (values '((content-type . (text/plain)))
	  "Hello, world!"))

(define my-sock (socket PF_UNIX SOCK_STREAM 0))
(bind my-sock AF_UNIX "/tmp/taubatron.sock")

(run-server dumb-handler 'http `(#:socket ,my-sock))
