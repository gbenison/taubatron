(use-modules (web request)
	     (web uri)
	     (web client)
	     (rnrs bytevectors)
	     (ice-9 rdelim))

(define uri (build-uri 'http
		       #:host "localhost"
		       #:port 8081
		       #:path "/taubatron"))

(define (make-post-request uri content)
  (build-request uri
		 #:method "POST"
		 #:headers `((content-length . ,(string-length content)))))

(define (send-post-request uri content)
  (let ((socket (open-socket-for-uri uri))
	(request (make-post-request uri content)))
    (write-request-body 
     (write-request request socket)
     (string->utf8 content))
    (force-output socket)
    (let* ((response (read-response socket))
	   (body (read-response-body response)))
      body)))