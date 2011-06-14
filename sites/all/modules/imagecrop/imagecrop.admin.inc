<?php
// $Id: imagecrop.admin.inc,v 1.1.4.16 2011/01/20 15:50:51 zuuperman Exp $

/**
 * @file
 * Administration functions for Imagecrop
 *
 * @date
 * Nov 3, 2010
 */

/**
 * Imagecrop settings page
 */
function imagecrop_settings() {

  // hook into image module
  if (module_exists('image')) {
    $options_modules['image'] = t('Hook into image module');
  }

  if (variable_get('user_pictures', 0)) {
    $options_modules['profile_picture'] = t('Hook into profile picture');
  }

  // hook into node_images module
  if (module_exists('node_images')) {
    $result = db_query("SELECT name,value FROM {variable} WHERE name LIKE 'node_images_position_%'");
    if ($result) {
      drupal_set_message(t('When you want to enable support for the node_images module, please read the README that comes with the imagecrop module.'));
      while ($row = db_fetch_object($result)) {
        if (variable_get($row->name, 'hide') != 'hide') {
          $explode = explode('_', $row->name);
          $options_modules[$row->name] = t('Hook into node_images module for <em>content type @type</em>', array('@type' => $explode[3]));
        }
      }
    }
  }

  // show checkboxes if options are not empty
  if (!empty($options_modules)) {
    $form['imagecrop_modules'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Hook into modules'),
      '#default_value' => variable_get('imagecrop_modules', array()),
      '#options' => $options_modules,
    );
    $form['array_filter'] = array('#type' => 'hidden');
  }
  else {
    $form['no_modules_fields'] = array(
      '#type' => 'item',
      '#value' => t('No supported modules were found to hook into.'),
    );
  }

  if (module_exists('imagefield')) {
    $form['imagefield'] = array(
      '#type' => 'item',
      '#value' => t('Imagecrop settings for CCK Imagefields are found on the field configuration pages.'),
    );
  }

  $all_popups = array('basic' => t('Basic popup window'));
  foreach (module_implements('imagecrop_popups') as $module_name) {

    $function = $module_name .'_imagecrop_popups';
    $popups = $function();
    if (is_array($popups)) {
      $all_popups += $popups;
    }

  }

  $form['imagecrop_popup'] = array(
    '#type' => 'radios',
    '#title' => t('Popup window type'),
    '#default_value' => variable_get('imagecrop_popup', 'basic'),
    '#options' => $all_popups,
  );

  $form['imagecrop_popup_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Popup window width'),
    '#default_value' => variable_get('imagecrop_popup_width', 700),
    '#size' => 4,
    '#field_suffix' => 'pixels',
  );

  $form['imagecrop_popup_height'] = array(
    '#type' => 'textfield',
    '#title' => t('Popup window height'),
    '#default_value' => variable_get('imagecrop_popup_height', 600),
    '#size' => 4,
    '#field_suffix' => 'pixels',
  );

  $form['imagecrop_scale_step'] = array(
    '#type' => 'textfield',
    '#title' => t('Step size for scale dropdown'),
    '#default_value' => variable_get('imagecrop_scale_step', 50),
    '#size' => 4,
    '#field_suffix' => 'pixels',
    '#description' => t('Setting the step to 0, will hide the dropdown'),
  );

  // drupal message if no action is found with javascript_crop
  if (count(imagecrop_presets_list()) == 0) {
    drupal_set_message(t('No preset is found with the javascript_crop action so far. If you want to take advantage of this module, you will need to create at least one preset with that action.'));
  }

  return system_settings_form($form);

}

/**
 * Show the cropped image.
 *
 * @param $fid file id
 * @param $presetid id of preset
 * @return cropped version of chosen image
 */
function imagecrop_showcrop($fid, $presetid = '', $module = '', $field = '', $node_type = '') {

  imagecrop_markup(FALSE, TRUE);
  $presets = return_presets($presetid, $module, $field, $node_type);

  if (count($presets) == 0) {
    return '<div id="imagecrop_info"  class="imagecrop_error">'. t('No preset is found with the javascript_crop action so far. If you want to take advantage of this module, you will need to create at least one preset with that action.') .'</div>';
  }

  $presetid = $presets['presetid'];
  $file = create_image_object($fid, $presetid, $module, TRUE);

  if (!$file) {
    return '<div id="imagecrop_info" class="imagecrop_error">'. t('Image to crop was not found.') .'</div>';
  }

  $output = theme('presettabs', $presets, $fid, $presetid, $module, $field, $node_type);
  $module = (!empty($module)) ? '/'. $module : '';
  $field = (!empty($field)) ? '/'. $field : '';
  $node_type = (!empty($node_type)) ? '/'. $node_type : '';
  $output .= '<div id="imagecrop_help">'. l(t('Click here to choose another crop area for this picture'), 'imagecrop/docrop/'. $fid .'/'. $presetid . $module . $field . $node_type) .'</div>';
  $output .= theme('imagecrop_result', $file->presetname, $file->filepath);
  return $output;

}

/**
 * Callback with javascript crop.
 *
 * @param $fid id of file
 * @param $presetid id of preset
 */
function imagecrop_docrop($fid, $presetid, $module = '', $field = '', $node_type = '') {

  imagecrop_markup(TRUE, TRUE);
  $presets = return_presets($presetid, $module, $field, $node_type);

  // no presets found for imagecrop
  if (count($presets) == 0) {
    return '<div id="imagecrop_info"  class="imagecrop_error">'. t('No preset is found with the javascript_crop action so far. If you want to take advantage of this module, you will need to create at least one preset with that action.') .'</div>';
  }

  $file = create_image_object($fid, $presetid, $module);

  // no image found
  if (!$file) {
    return '<div id="imagecrop_info" class="imagecrop_error">'. t('Image to crop was not found.') .'</div>';
  }

  $size_warning = FALSE;
  // get size of temporary image
  $size = getimagesize($file->dst);
  $width = $size[0];
  $height = $size[1];
  // return warning message if crop toolbox is too big and not resizable.
  if (($width < $file->crop_width || $height < $file->crop_height) && $file->resizable == 0) {
    $size_warning = FALSE;
  }

  // add jquery ui
  if ($file->resizable) {

    jquery_ui_add(array('ui.resizable', 'ui.draggable', 'effects.scale'));
    $aspect = FALSE;

    // set aspect ration if needed
    if ($file->aspect) {
      if (is_numeric($file->aspect)) {
        $aspect = $file->aspect;
      }
    }

    // add imagecrop aspect setting
    drupal_add_js(array('imagecrop' => array('aspectRatio' => $aspect)), 'setting');

  }
  else {
    jquery_ui_add(array('ui.draggable'));
  }

  // output
  if ($size_warning == FALSE) {
    $url = file_create_url($file->dst);
    $url .= strstr($url, '?')? '&time='. time() : '?time='. time();
    $output = theme('presettabs', $presets, $fid, $presetid, $module, $field, $node_type);
    $output .= '<div id="imagecrop_help">'. t("Resize image if needed, then select a crop area. Click 'Crop image thumbnail' to save your crop area.<br/>After that close the window and save your node form.") .'</div>';
    $output .= theme('imagecrop', $url, $width, $height, $file->resizable);
    $output .= drupal_get_form('imageoffsets_form', $file, $presetid, $fid, $module, $field, $node_type);
  }
  else {
    $output .= '<div id="imagecrop_info" class="imagecrop_error">'. t('The crop toolbox is too big for this image.') .' <a href="javascript:history.back();"><span class="white">'. t('Back') .'</span></a></div>';
  }

  return $output;

}

/**
 * Callback to return offsets, height & width
 *
 * @param $file current file being cropped
 * @param $presetid id of preset
 * @param $fid id of file
 * @param $module specific module
 * @return array $form
 */
function imageoffsets_form(&$form_state, $file, $presetid, $fid, $module, $field, $node_type) {

  $step = variable_get('imagecrop_scale_step', 50);
  $aspect = $file->orig_width / $file->orig_height;

  $options = array();
  if ($step > 0) {
    $original_width = $file->orig_width;
    $options['original'] = $file->orig_width .' x '. $file->orig_height .'px (Original)';
    $file->orig_width -= $step;

    while ($file->orig_width > $file->crop_width && ($file->orig_width / $aspect) > $file->crop_height) {
      $options[$file->orig_width] = $file->orig_width .' x '. intval($file->orig_width / $aspect) .'px ('. round((($file->orig_width / $original_width) * 100), 2) .'%)';
      $file->orig_width -= $step;
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Crop image thumbnail'),
    '#prefix' => '<table id="imagecrop_table_actions"><tr><td>',
    '#suffix' => '</td>',
  );

  // only show when there are multiple scaling options available
  if (count($options) > 1) {
    $form['scaling'] = array(
      '#type' => 'select',
      '#options' => $options,
      '#default_value' => $file->scale,
      '#prefix' => '<td id="imagecrop-throbber">',
      '#suffix' => '</td></tr></table>',
      '#attributes' => array('onchange' => "$('#imagecrop-throbber').addClass('show'); $('#edit-scaledown').click();"),
    );
    $form['scaledown'] = array(
      '#type' => 'submit',
      '#value' => t('Scale image'),
      '#attributes' => array('style' => "display:none;"),
    );
  }

  // load crop from other presets
/*  $result = db_query('SELECT ic.name, ic.presetid FROM {imagecrop} ic INNER JOIN {imagecache_preset} ip ON ip.presetid = ic.presetid WHERE fid = %d', $fid);
  $form['crop-selection'] = array(
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => $scale,
    '#prefix' => '<td id="imagecrop-throbber2">',
    '#suffix' => '</td></tr></table>',
    '#attributes' => array('onchange' => "$('#imagecrop-throbber2').addClass('show'); $('#load-crop').click();"),
  );
  $form['load-crop'] = array(
    '#type' => 'submit',
    '#value' => t('Scale image'),
    '#attributes' => array('style' => "display:none;"),
  );*/

  $form['image-crop-x'] = array(
    '#type' => 'hidden',
    '#default_value' => $file->xoffset,
    '#attributes' => array('class' => 'edit-image-crop-x'),
  );

  $form['image-crop-y'] = array(
    '#type' => 'hidden',
    '#default_value' => $file->yoffset,
    '#attributes' => array('class' => 'edit-image-crop-y'),
  );

  $form['image-crop-width'] = array(
    '#type' => 'hidden',
    '#default_value' => $file->crop_width,
    '#attributes' => array('class' => 'edit-image-crop-width'),
  );

  $form['image-crop-height'] = array(
    '#type' => 'hidden',
    '#default_value' => $file->crop_height,
    '#attributes' => array('class' => 'edit-image-crop-height'),
  );

  $form['fid'] = array(
    '#type' => 'hidden',
    '#value' => $fid,
  );

  $form['module'] = array(
    '#type' => 'hidden',
    '#value' => $module,
  );

  $form['field'] = array(
    '#type' => 'hidden',
    '#value' => $field,
  );

  $form['node_type'] = array(
    '#type' => 'hidden',
    '#value' => $node_type,
  );

  $form['presetid'] = array(
    '#type' => 'hidden',
    '#value' => $presetid,
  );

  $form['preset-destination'] = array(
    '#type' => 'hidden',
    '#value' => $file->preset_destination
  );

  $form['temp-preset-destination'] = array(
    '#type' => 'hidden',
    '#value' => $file->dst
  );

  return $form;

}

/**
 * Save the offset & size values
 *
 * @param $form_id id of the form
 * @param $form_values submitted values of the imageoffsets form
 */
function imageoffsets_form_submit($form, &$form_state) {

  if ($form_state['values']['op'] == t('Scale image')) {
    $form_state['values']['image-crop-x'] = 0;
    $form_state['values']['image-crop-y'] = 0;
  }

  $form_state['values']['image-crop-x'] = max($form_state['values']['image-crop-x'], 0);
  $form_state['values']['image-crop-y'] = max($form_state['values']['image-crop-y'], 0);

  // save into imagecrop tables
  $module = (!empty($form_state['values']['module'])) ? '/'. $form_state['values']['module'] : '';
  $field = (!empty($form_state['values']['field'])) ? '/'. $form_state['values']['field'] : '';
  $node_type = (!empty($form_state['values']['node_type'])) ? '/'. $form_state['values']['node_type'] : '';
  $reference = (!empty($form_state['values']['module']) && ($form_state['values']['module'] == 'node_images' || $form_state['values']['module'] == 'user')) ? $form_state['values']['module'] : 'files';
  $scaling = (isset($form_state['values']['scaling']) ? $form_state['values']['scaling'] : 'original');

  db_query("DELETE FROM {imagecrop} WHERE fid = %d AND presetid = %d AND reference = '%s'", $form_state['values']['fid'], $form_state['values']['presetid'], $reference);
  db_query("INSERT INTO {imagecrop} VALUES (%d, %d, '%s', %d, %d, %d, %d, '%s')", $form_state['values']['fid'], $form_state['values']['presetid'], $reference, $form_state['values']['image-crop-x'], $form_state['values']['image-crop-y'], $form_state['values']['image-crop-width'], $form_state['values']['image-crop-height'], $scaling);

  if ($form_state['values']['op'] == t('Scale image')) {
    drupal_goto('imagecrop/docrop/'. $form_state['values']['fid'] .'/'. $form_state['values']['presetid'] . $module . $field . $node_type);
  }
  else {
    // delete imagecache preset, so newest file is generated.
    file_delete($form_state['values']['preset-destination']);
    if ($form_state['values']['preset-destination'] != $form_state['values']['temp-preset-destination']) {
      file_delete($form_state['values']['temp-preset-destination']);
    }
    drupal_goto('imagecrop/showcrop/'. $form_state['values']['fid'] .'/'. $form_state['values']['presetid'] . $module . $field . $node_type);
  }

}