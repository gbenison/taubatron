function tweetenable (d,s,id){
      var js,fjs=d.getElementsByTagName(s)[0];
      if(!d.getElementById(id)){js=d.createElement(s);
				js.id=id;
				js.src="//platform.twitter.com/widgets.js";
				fjs.parentNode.insertBefore(js,fjs);}}

function tweetinit() { tweetenable(document,"script","twitter-wjs"); }

