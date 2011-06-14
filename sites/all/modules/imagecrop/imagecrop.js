/* $Id: imagecrop.js,v 1.1.4.11 2011/01/18 21:55:13 zuuperman Exp $ */

/*
 * Toolbox @copyright from imagefield_crop module with some minor modifications.
 * To be used with Jquery UI.
 */

$(document).ready(function(){

	if ($('#resizeMe').resizable) {	
	
	  $('#resizeMe').resizable({
		containment: $('#image-crop-container'),
		//transparent: true,
		aspectRatio: Drupal.settings.imagecrop.aspectRatio,
		autohide: true,
		handles: 'n, e, s, w, ne, se, sw, nw',

		resize: function(e, ui) {
		  
      var left = (ui.position.left > 0) ? ui.position.left : 0;
      var top = (ui.position.top > 0) ? ui.position.top : 0;
      this.style.backgroundPosition = '-' + left + 'px -' + top + 'px';
      $("#edit-image-crop-x").val(left);
      $("#edit-image-crop-y").val(top);		  
      
		  $("#edit-image-crop-width").val($('#resizeMe').width());
		  $("#edit-image-crop-height").val($('#resizeMe').height());
		  
		},
		stop: function(e, ui) {
  		  this.style.backgroundPosition = '-' + (ui.position.left) + 'px -' + (ui.position.top) + 'px';
  	    }
	  });
	  
	}

	$('#resizeMe').draggable({
		cursor: 'move',
		containment: $('#image-crop-container'),
		drag: function(e, ui) {
		  var left = (ui.position.left > 0) ? ui.position.left : 0;
		  var top = (ui.position.top > 0) ? ui.position.top : 0;
		  this.style.backgroundPosition = '-' + left + 'px -' + top + 'px';
		  $("#edit-image-crop-x").val(left);
		  $("#edit-image-crop-y").val(top);
		}
	});
	
	$('#image-crop-container').css({ opacity: 0.5 });
	$('#resizeMe').css({ position : 'absolute' });
	
    var leftpos = $('#edit-image-crop-x').val();
    var toppos = $('#edit-image-crop-y').val();
    $("#resizeMe").css({backgroundPosition: '-'+ leftpos + 'px -'+ toppos +'px'});
    $("#resizeMe").width($('#edit-image-crop-width').val() + 'px');
    $("#resizeMe").height($('#edit-image-crop-height').val() + 'px');
    $("#resizeMe").css({top: $('#edit-image-crop-y').val() +'px' });
    $("#resizeMe").css({left: $('#edit-image-crop-x').val() +'px' });
    
});
