<?php
// $Id: theme.inc,v 1.1.2.2 2010/04/15 23:56:03 inadarei Exp $
/**
 * @file
 * OP Author Features Theming
 */


/**
* Implementation of hook_init
*/
function op_slideshow_init() {
  if (_op_slideshow_is_slideshow_page()) {
 
    $module_path = drupal_get_path('module', 'op_slideshow');
        
    //-- Load Author-page-specific CSS
    $css_path = base_path() . $module_path . '/css/slideshow.css';
    drupal_set_html_head("<link type=\"text/css\" rel=\"stylesheet\" media=\"all\" href=\"$css_path?nocache=" . time() . "\" />");
  }

}

/**
* Implementation of hook_registry_alter
*/
function op_slideshow_theme_registry_alter(&$theme_registry) {

  //-- Provide default node template implementations from this module,
  //-- but make sure we give a correspodning tpl in a theme folder a chance 
  //-- to override us.
  $idx = array_search('modules/node', $theme_registry['node']['theme paths']);
  if ($idx !== FALSE) {
    array_splice( $theme_registry['node']['theme paths'], 
                  $idx+1, 0, 
                  drupal_get_path('module', 'op_slideshow') . '/theme');
  }
                
}


/**
* Implementation of hook_preprocess_page
*/
function op_slideshow_preprocess_page(&$vars) {

  if (_op_slideshow_is_slideshow_page()) {  
    //-- Unset title template variable for slideshow pages, to make sure page.tpl.php does not render it.
  //  unset($vars['title']); 
  }
  
}

/**
* Checks if current page is an slideshow node page
*/
function _op_slideshow_is_slideshow_page() {

  static  $is_slideshow_page;  
  if (isset($is_slideshow_page)) return $is_slideshow_page;
  
  $is_slideshow_page = FALSE;
  
  if (arg(0) == 'node' && is_numeric(arg(1))) { // Node page
    $node_id = arg(1);
    $node = node_load($node_id);
    $is_slideshow_page = ($node->type == 'slideshow');
  }
  return $is_slideshow_page;
  
}

function op_slideshow_theme($existing) {  
  $themes = array();
 
  // — Row-level theme for stories
  $key = 'views_view_fields__image_gallery_images__block_1';
  $tpl_path = 'theme/' . str_replace('_', '-', $key); 
 
  $themes[$key] = array (
    'arguments' => array('view' => Null, 'options' => Null, 'row' => Null),
    'template' => $tpl_path,
    'original hook' => 'views_view_fields',
    
    // — According to Views Advanced Help, we either need to do this or make module weight > 10.
    'preprocess functions' => array(
          'template_preprocess',
          'template_preprocess_views_view_fields',
          'op_author_layout_preprocess_views_view_fields',
    ),
  );
  return $themes;
}
