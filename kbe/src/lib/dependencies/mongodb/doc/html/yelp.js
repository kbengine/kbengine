
var __yelp_generate_id_counter__ = 0;
function yelp_generate_id () {
  var ret = 'yelp--' + (++__yelp_generate_id_counter__).toString();
  if ($('#' + ret).length != 0)
    return yelp_generate_id();
  else
    return ret;
};
$(document).ready (function () {
  var highlight_hash = function () {
    if (location.hash != '') {
      var sect = $(location.hash);
      sect.css('background-color',  '#fffacc');
      window.setTimeout(function () {
        sect.css({
          '-webkit-transition': 'background-color 2s linear',
          '-moz-transition': 'background-color 2s linear',
          'transition': 'background-color 2s linear',
          'background-color': 'rgba(1, 1, 1, 0)'
        });
      }, 200);
    }
  };
  $(window).bind('hashchange', highlight_hash);
  highlight_hash();
});

$.fn.yelp_ui_expander_toggle = function (onlyopen, callback) {
  var expander = $(this);
  var region = expander.children('.inner').children('.region');
  var yelpdata = expander.children('div.yelp-data-ui-expander');
  var compfunc = function () {
    if (expander.is('div.figure')) { expander.yelp_auto_resize(); }
    if (callback) { callback(); }
    return true;
  };
  var title = expander.children('.inner').children('.title');
  if (title.length == 0)
    title = expander.children('.inner').children('.hgroup');
  var title = title.find('span.title:first');
  if (expander.is('.ui-expander-e')) {
    if (!onlyopen) {
      expander.removeClass('ui-expander-e').addClass('ui-expander-c');
      region.attr('aria-expanded', 'false').slideUp('fast', compfunc);
      title.html(yelpdata.children('div.yelp-title-collapsed').html());
    }
  }
  else {
    expander.removeClass('ui-expander-c').addClass('ui-expander-e');
    region.attr('aria-expanded', 'true').slideDown('fast', compfunc);
    title.html(yelpdata.children('div.yelp-title-expanded').html());
  }
};
$(document).ready(function () {
  $('.ui-expander').each(function () {
    var expander = $(this);
    var yelpdata = expander.children('div.yelp-data-ui-expander');
    var region = expander.children('.inner').children('.region');
    var title = expander.children('.inner').children('.title');
    var issect = false;
    if (title.length == 0) {
      title = expander.children('.inner').children('.hgroup');
      issect = true;
    }
    if (title.length == 0) {
      return;
    }
    if (region.attr('id') == '')
      region.attr('id', yelp_generate_id());
    title.attr('role', 'button').attr('aria-controls', region.attr('id'));
    var titlespan = title.find('span.title:first');
    var title_e = yelpdata.children('div.yelp-title-expanded');
    var title_c = yelpdata.children('div.yelp-title-collapsed');
    if (title_e.length == 0)
      yelpdata.append($('<div class="yelp-title-expanded"></div>').html(titlespan.html()));
    if (title_c.length == 0)
      yelpdata.append($('<div class="yelp-title-collapsed"></div>').html(titlespan.html()));
    if (yelpdata.attr('data-yelp-expanded') == 'false') {
      expander.addClass('ui-expander-c');
      region.attr('aria-expanded', 'false').hide();
      if (title_c.length != 0)
        titlespan.html(title_c.html());
    } else {
      expander.addClass('ui-expander-e');
      region.attr('aria-expanded', 'true');
      if (title_e.length != 0)
        titlespan.html(title_e.html());
    }
    title.click(function () {
      expander.yelp_ui_expander_toggle(false);
    });
  });
});
$(document).ready(function () {
  var expand_hash = function () {
    if (location.hash != '') {
      var target = $(location.hash);
      var parents = target.parents('div.ui-expander');
      if (target.is('div.ui-expander'))
        parents = parents.andSelf();
      parents.each(function () {
        $(this).yelp_ui_expander_toggle(true, function () {
          window.scrollTo(0, $(target).offset().top);
        });
      });
    }
  };
  $(window).bind('hashchange', expand_hash);
  expand_hash();
});

yelp_color_text_light = '#2e3436';
yelp_color_gray_background = '#f3f3f0';
yelp_color_gray_border = '#babdb6';
yelp_paint_zoom = function (zoom, zoomed) {
  var ctxt = zoom.children('canvas')[0].getContext('2d');
  ctxt.strokeStyle = ctxt.fillStyle = yelp_color_text_light;
  ctxt.clearRect(0, 0, 10, 10);
  ctxt.strokeRect(0.5, 0.5, 9, 9);
  if (zoomed) {
    ctxt.fillRect(1, 1, 9, 4);
    ctxt.fillRect(5, 5, 4, 4);
    zoom.attr('title', zoom.attr('data-zoom-out-title'));
  }
  else {
    ctxt.fillRect(1, 5, 4, 4);
    zoom.attr('title', zoom.attr('data-zoom-in-title'));
  }
}
$.fn.yelp_auto_resize = function () {
  var fig = $(this);
  if (fig.is('img'))
    fig = fig.parents('div.figure').eq(0);
  if (fig.data('yelp-zoom-timeout') != undefined) {
    clearInterval(fig.data('yelp-zoom-timeout'));
    fig.removeData('yelp-zoom-timeout');
  }
  var imgs = fig.find('img');
  for (var i = 0; i < imgs.length; i++) {
    var img = $(imgs[i]);
    if (img.data('yelp-load-bound') == true)
      img.unbind('load', fig.yelp_auto_resize);
    if (!imgs[i].complete) {
      img.data('yelp-load-bound', true);
      img.bind('load', fig.yelp_auto_resize);
      return false;
    }
  }
  $(window).unbind('resize', yelp_resize_imgs);
  var zoom = fig.children('div.inner').children('a.zoom');
  if (fig.find('div.contents:first').is(':hidden')) {
    zoom.hide();
    return;
  }
  for (var i = 0; i < imgs.length; i++) {
    var img = $(imgs[i]);
    if (img.data('yelp-original-width') == undefined) {
      var iwidth = parseInt(img.attr('width'));
      if (!iwidth)
        iwidth = img[0].width;
      img.data('yelp-original-width', iwidth);
      var iheight = parseInt(img.attr('height'));
      if (!iheight)
        iheight = img[0].height * (iwidth / img[0].width);
      img.data('yelp-original-height', iheight);
    }
    if (img.data('yelp-original-width') > img.parent().width()) {
      if (img.data('yelp-zoomed') != true) {
        img[0].width = img.parent().width();
        img[0].height = (parseInt(img.data('yelp-original-height')) *
                         img.width() / parseInt(img.data('yelp-original-width')));
      }
      zoom.show();
    }
    else {
      img[0].width = img.data('yelp-original-width');
      img[0].height = img.data('yelp-original-height');
      zoom.hide();
    }
  }
  /* The image scaling above can cause the window to resize if it causes
   * scrollbars to disappear or reapper. Unbind the resize handler before
   * scaling the image. Don't rebind immediately, because we'll still get
   * that resize event in an idle. Rebind on the callback instead.
   */
  var reresize = function () {
    $(window).unbind('resize', reresize);
    $(window).bind('resize', yelp_resize_imgs);
  }
  $(window).bind('resize', reresize);
  return false;
};
yelp_resize_imgs = function () {
  $('div.figure img').parents('div.figure').each(function () {
    var div = $(this);
    if (div.data('yelp-zoom-timeout') == undefined)
      div.data('yelp-zoom-timeout', setTimeout(function () { div.yelp_auto_resize() }, 1));
  });
  return false;
};
$(document).ready(function () {
  $('div.figure img').parents('div.figure').each(function () {
    var fig = $(this);
    var zoom = fig.children('div.inner').children('a.zoom');
    zoom.append($('<canvas width="10" height="10"/>'));
    yelp_paint_zoom(zoom, false);
    zoom.data('yelp-zoomed', false);
    zoom.click(function () {
      var zoomed = !zoom.data('yelp-zoomed');
      zoom.data('yelp-zoomed', zoomed);
      zoom.parent().find('img').each(function () {
        var zimg = $(this);
        zimg.data('yelp-zoomed', zoomed);
        if (zoomed) {
          zimg[0].width = zimg.data('yelp-original-width');
          zimg[0].height = zimg.data('yelp-original-height');
        } else {
          zimg[0].width = zimg.parent().width();
          zimg[0].height = (parseInt(zimg.data('yelp-original-height')) *
                            zimg.width() / parseInt(zimg.data('yelp-original-width')));
        }
        yelp_paint_zoom(zoom, zoomed);
      });
      return false;
    });
  });
  yelp_resize_imgs();
  $(window).bind('resize', yelp_resize_imgs);
});
function yelp_init_video (element) {
  var video = $(element);
  video.removeAttr('controls');

  var controls = $('<div class="media-controls media-controls-' + element.nodeName + '"></div>');
  var playControl = $('<button class="media-play"></button>').attr({
    'data-play-label': video.attr('data-play-label'),
    'data-pause-label': video.attr('data-pause-label'),
    'value': video.attr('data-play-label')
  });
  var playCanvas = $('<canvas width="20" height="20"></canvas>');
  playControl.append(playCanvas);
  var rangeCanvas = $('<canvas width="104" height="20"></canvas>');
  var rangeCanvasCtxt = rangeCanvas[0].getContext('2d');
  rangeCanvasCtxt.strokeStyle = yelp_color_gray_border;
  rangeCanvasCtxt.strokeWidth = 1;
  rangeCanvasCtxt.strokeRect(0.5, 0.5, 103, 19);
  var currentSpan = $('<span class="media-current">0:00</span>');
  var durationSpan = $('<span class="media-duration">-:--</span>');
  controls.append(playControl,
                  $('<div class="media-range"></div>').append(rangeCanvas),
                  $('<div class="media-time"></div>').append(currentSpan, durationSpan));
  video.after(controls);

  var playCanvasCtxt = playCanvas[0].getContext('2d');
  var paintPlayButton = function () {
    playCanvasCtxt.fillStyle = yelp_color_gray_background;
    playCanvasCtxt.clearRect(0, 0, 20, 20);
    playCanvasCtxt.beginPath();
    playCanvasCtxt.moveTo(5, 5);   playCanvasCtxt.lineTo(5, 15);
    playCanvasCtxt.lineTo(15, 10); playCanvasCtxt.lineTo(5, 5);
    playCanvasCtxt.fill();
  }
  var paintPauseButton = function () {
    playCanvasCtxt.fillStyle = yelp_color_gray_background;
    playCanvasCtxt.clearRect(0, 0, 20, 20);
    playCanvasCtxt.beginPath();
    playCanvasCtxt.moveTo(5, 5);   playCanvasCtxt.lineTo(9, 5);
    playCanvasCtxt.lineTo(9, 15);  playCanvasCtxt.lineTo(5, 15);
    playCanvasCtxt.lineTo(5, 5);   playCanvasCtxt.fill();
    playCanvasCtxt.beginPath();
    playCanvasCtxt.moveTo(11, 5);  playCanvasCtxt.lineTo(15, 5);
    playCanvasCtxt.lineTo(15, 15); playCanvasCtxt.lineTo(11, 15);
    playCanvasCtxt.lineTo(11, 5);  playCanvasCtxt.fill();
  }
  paintPlayButton();

  var video_el = video[0];
  var mediaChange = function () {
    if (video_el.ended)
      video_el.pause()
    if (video_el.paused) {
      playControl.attr('value', playControl.attr('data-play-label'));
      paintPlayButton();
    }
    else {
      playControl.attr('value', playControl.attr('data-pause-label'));
      paintPauseButton();
    }
  }
  video_el.addEventListener('play', mediaChange, false);
  video_el.addEventListener('pause', mediaChange, false);
  video_el.addEventListener('ended', mediaChange, false);

  var playClick = function () {
    if (video_el.paused || video_el.ended)
      video_el.play();
    else
      video_el.pause();
  };
  playControl.click(playClick);

  var ttmlDiv = video.parent().children('div.media-ttml');
  var ttmlNodes = ttmlDiv.find('.media-ttml-node');

  var timeUpdate = function () {
    var pct = (element.currentTime / element.duration) * 100;
    rangeCanvasCtxt.fillStyle = yelp_color_gray_border;
    rangeCanvasCtxt.clearRect(2, 2, 100, 16);
    rangeCanvasCtxt.fillRect(2, 2, pct, 16);
    var mins = parseInt(element.currentTime / 60);
    var secs = parseInt(element.currentTime - (60 * mins))
    currentSpan.text(mins + (secs < 10 ? ':0' : ':') + secs);
    ttmlNodes.each(function () {
      var ttml = this;
      if (element.currentTime >= parseFloat(ttml.getAttribute('data-ttml-begin')) &&
          (!ttml.hasAttribute('data-ttml-end') ||
           element.currentTime < parseFloat(ttml.getAttribute('data-ttml-end')) )) {
        if (ttml.tagName == 'span')
          ttml.style.display = 'inline';
        else
          ttml.style.display = 'block';
      }
      else {
        ttml.style.display = 'none';
      }
    });
  };
  element.addEventListener('timeupdate', timeUpdate, false);
  var durationUpdate = function () {
    if (!isNaN(element.duration)) {
      mins = parseInt(element.duration / 60);
      secs = parseInt(element.duration - (60 * mins));
      durationSpan.text(mins + (secs < 10 ? ':0' : ':') + secs);
    }
  };
  element.addEventListener('durationchange', durationUpdate, false);

  rangeCanvas.click(function (event) {
    var pct = event.clientX - Math.floor(rangeCanvas.offset().left);
    if (pct < 0)
      pct = 0;
    if (pct > 100)
      pct = 100;
    element.currentTime = (pct / 100.0) * element.duration;
  });
};
$(document).ready(function () {
  $('video, audio').each(function () { yelp_init_video(this) });;
});

$(document).ready( function () { jQuery.syntax({root: '', blockLayout: 'yelp',
theme: false, linkify: false}); });

$(document).ready(function () {
  $('input.facet').change(function () {
    var control = $(this);
    var content = control.closest('div.body,div.sect');
    content.find('.facet-link').each(function () {
      var link = $(this);
      var facets = link.parents('div.body,div.sect').children('div.region').children('div.contents').children('div.facets').children('div.facet');
      var visible = true;
      for (var i = 0; i < facets.length; i++) {
        var facet = facets.slice(i, i + 1);
        var facetvis = false;
        var inputs = facet.find('input.facet:checked');
        for (var j = 0; j < inputs.length; j++) {
          var input = inputs.slice(j, j + 1);
          var inputvis = false;
          var key = input.attr('data-facet-key');
          var values = input.attr('data-facet-values').split(' ');
          for (var k = 0; k < values.length; k++) {
            if (link.is('*[data-facet-' + key + ' ~= "' + values[k] + '"]')) {
              inputvis = true;
              break;
            }
          }
          if (inputvis) {
            facetvis = true;
            break;
          }
        }
        if (!facetvis) {
          visible = false;
          break;
        }
      }
      if (!visible)
        link.hide('fast');
      else
        link.show('fast');
    });
  });
});

$(document).ready(function () {
  $('a.gloss-term').each(function () {
    if ($(this).attr('href') == '#') {
      $(this).click(function () { return false; });
    }
    var showtip = function () {
      var desc = $(this).children('span.gloss-desc');
      if (desc.is(':visible'))
        return;
      var top = $(this).offset().top + $(this).height() + 1;
      var left = $(this).offset().left;
      var cnt = $(this).closest('div.contents');
      var diff = cnt.offset().left + cnt.width() - desc.width() - 4;
      if (left > diff)
        left = diff;
      desc.css({'top': top + 'px', 'left': left + 'px'}).fadeIn('slow');
    };
    var hidetip = function () {
      if ($(this).is(':focus'))
        return;
      $(this).children('span.gloss-desc').fadeOut('fast');
    };
    $(this).hover(showtip, hidetip);
    $(this).focus(showtip);
    $(this).blur(hidetip);
  });
});

$(document).ready(function () {
  $('div.mouseovers').each(function () {
    var contdiv = $(this);
    var width = 0;
    var height = 0;
    contdiv.find('img').each(function () {
      if ($(this).attr('data-yelp-match') == '')
        $(this).show();
    });
    contdiv.next('ul').find('a').each(function () {
      var mlink = $(this);
      mlink.hover(
        function () {
          if (contdiv.is(':visible')) {
            var offset = contdiv.offset();
            mlink.find('img').css({left: offset.left, top: offset.top, zIndex: 10});
            mlink.find('img').fadeIn('fast');
          }
        },
        function () {
          mlink.find('img').fadeOut('fast');
        }
      );
    });
  });
  $('div.links-ui-hover').each(function () {
    var contdiv = $(this);
    var width = 0;
    var height = 0;
    contdiv.next('ul').find('a').each(function () {
      var mlink = $(this);
      mlink.hover(
        function () {
          if (contdiv.is(':visible')) {
            var offset = contdiv.offset();
            mlink.find('img').parent('span').css({left: offset.left, top: offset.top, zIndex: 10});
            mlink.find('img').parent('span').show();
          }
        },
        function () {
          mlink.find('img').parent('span').hide();
        }
      );
    });
  });
  $('a.ui-overlay').each(function () {
    $(this).click(function () {
      var overlay = $(this).parent('div').children('div.ui-overlay');
      var inner = overlay.children('div.inner');
      var close = inner.children('a.ui-overlay-close');
      var media = inner.find('audio, video');
      var screen = $('div.ui-screen');
      if (screen.length == 0) {
        screen = $('<div class="ui-screen"></div>');
        $('body').append(screen);
      }
      var hideoverlay = function () {
        if (media.length > 0)
          media[0].pause();
        $(document).unbind('keydown.yelp-ui-overlay');
        close.unbind('click');
        screen.unbind('click');
        screen.fadeOut('slow');
        overlay.unbind('click');
        overlay.slideUp('fast');
        return false;
      };
      screen.click(hideoverlay);
      close.click(hideoverlay);
      $(document).bind('keydown.yelp-ui-overlay', function (event) {
        if (event.which == 27) {
          hideoverlay();
        }
      });
      overlay.click(function (event) {
        var target = event.target;
        do {
          if (target == inner[0]) {
            break;
          }
        } while (target = target.parentNode);
        if (target != inner[0]) {
          hideoverlay();
          return false;
        }
      });
      screen.fadeIn('slow');
      overlay.slideDown('fast', function () {
        if (media.length > 0)
          media[0].play();
      });
      return false;
    });
  });
});
