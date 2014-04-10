<div<?php print $attributes; ?>>
  <header class="l-header" role="banner">
    <script src="//ajax.googleapis.com/ajax/libs/jquery/1.10.1/jquery.min.js"></script>
  <script type="text/javascript">
$(function(){
   $('#btn_up').click(function() {
      $('html,body').animate({scrollTop: 0}, 'slow');
   });
  
   $(window).scroll(function(){
      if($(window).scrollTop()<350){
         $('#btn_up').fadeOut();
      }else{
         $('#btn_up').fadeIn();
      }
   });
});
</script>
      <?php print render($page['header']); ?>
    <div class="l-branding">
      <?php if ($logo): ?>
        <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home" class="site-logo"><img src="<?php print $logo; ?>" alt="<?php print t('Home'); ?>" /></a>
      <?php endif; ?>

      <?php if ($site_name || $site_slogan): ?>
        <?php if ($site_name): ?>
          <h1 class="site-name">
            <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home"><span><?php print $site_name; ?></span></a>
          </h1>
        <?php endif; ?>

        <?php if ($site_slogan): ?>
          <h2 class="site-slogan"><?php print $site_slogan; ?></h2>
        <?php endif; ?>
      <?php endif; ?>

      <?php print render($page['branding']); ?>
    </div>


    <?php print render($page['navigation']); ?>
  </header>

  <div class="l-main">
  <div id="fond-content">
  <div id="slide">
    <?php print render($page['highlighted']); ?>
    </div>


    <div class="l-content" role="main">
          <?php print render($page['content']); ?>
      
          </div>
      <a id="main-content"></a>
      <?php print render($title_prefix); ?>
      <?php print render($title_suffix); ?>
      <?php print $messages; ?>
      <?php print render($tabs); ?>
      <?php print render($page['help']); ?>
      <?php print $feed_icons; ?>
      <div id="youbook">
  <?php print render($page['sidebar_first']); ?>
      </div>

    </div>

  </div>
  <footer class="l-footer" role="contentinfo">
    <?php print render($page['footer']); ?>
  </footer>
</div>
<div id="btn_up">
   <img alt="Retour en haut" title="Retour en haut" src="http://www.dafygap.com/dev/sites/all/themes/dafy/images/goontop.png" />
</div>
