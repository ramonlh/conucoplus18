PGM_P(ajaxscript) =
  "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js'></script>"
  "<script>"
  "function gt(n){$.ajax({url:'tt',success:function(result){{$('#tt').html(result);}}});}"
  "function gl(n){$.ajax({url:'l'+n,success:function(result){{$('#l'+n).html(result);}}});}"
  "function gr(n){$.ajax({url:'r'+n,success:function(result){{$('#r'+n).html(result);}}});}"
  "function ch(){for(i=0;i<4;i++){gl(i);} for(i=0;i<0;i++){gr(i);}setTimeout(function(){ch();},1000);}"
  "function ch5(){for(i=0;i<1;i++){gt(i);} setTimeout(function(){ch5();},5000);}"
  "ch();ch5();"
  "</script>";

