#!/bin/sh
#-*- scheme -*-
#
#    Copyright (C) 2012 Greg Benison
#   
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#   
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#   
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#  

export GUILE_LOAD_PATH=$GUILE_LOAD_PATH:`pwd`;
export LTDL_LIBRARY_PATH=$LTDL_LIBRARY_PATH:`pwd`;
export GUILE_WARN_DEPRECATED=no;
# exec guile -s $0 2>/dev/null
exec guile -s $0 2>>guile-error.log

!#

(use-modules (taubatron)
	     (sxml simple)
	     (www cgi))

(cgi:init)

(define (safe-car xs)(false-if-exception (car xs)))
(define (as-li x)`(li ,x))

(define path-form
  (let ((start (safe-car (cgi:values "start")))
	(end   (safe-car (cgi:values "end"))))
    (if (and start end)
	(let ((path (find-path start end)))
	  `(ul ,(map as-li path)))
	"Enter starting and ending words below")))

(display "Content-type: text/html\n\n")

(sxml->xml
 `(html
   (head
    (title "t**tron")
    (body ,path-form
	  (form (@ (method "post")
		   (action "taubatron.cgi"))
		(input (@ (type "text")
			  (name "start")))
		(input (@ (type "text")
			  (name "end")))
		(input (@ (type "submit")
			  (value "Go"))))))))
	   