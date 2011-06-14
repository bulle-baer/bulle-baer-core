<?php
// $Id: imagecrop_actions.inc,v 1.1.4.15 2010/12/09 18:30:34 zuuperman Exp $

/**
 * @file
 * Imagecache actions implementation.
 *
 * @param $data values passed on by imagecache
 */

function imagecrop_javascript_form($data) {
  $form['width'] = array(
    '#type' => 'textfield',
    '#title' => t('Width'),
    '#required' => TRUE,
    '#default_value' => $data['width'],
    '#description' => t('Enter a width in pixels or as a percentage. i.e. 500 or 80%.'),
  );
  $form['height'] = array(
    '#type' => 'textfield',
    '#title' => t('Height'),
    '#required' => TRUE,
    '#default_value' => $data['height'],
    '#description' => t('Enter a height in pixels or as a percentage. i.e. 500 or 80%.'),
  );
  $form['xoffset'] = array(
    '#type' => 'textfield',
    '#title' => t('X offset'),
    '#default_value' => $data['xoffset'],
    '#description' => t('Enter an offset in pixels or use a keyword: <em>left</em>, <em>center</em>, or <em>right</em>.'),
  );
  $form['yoffset'] = array(
    '#type' => 'textfield',
    '#title' => t('Y offset'),
    '#default_value' => $data['yoffset'],
    '#description' => t('Enter an offset in pixels or use a keyword: <em>top</em>, <em>center</em>, or <em>bottom</em>.'),
  );
  $form['resizable'] = array(
    '#type' => 'checkbox',
    '#title' => t('Is the toolbox resizable or not?'),
    '#default_value' => $data['resizable'],
    '#description' => t('If the toolbox is resized, the crop values won\'t be respected, so you should add a Scale action after the ImageCrop.'),
  );

  $description = t('Enter an aspect ratio to preserve during resizing. This can take one of the following formats:');
  $description .= '<ul><li>'. t('A float (like 0.5 or 2).') .'</li>';
  $description .= '<li>'. t('The string \'KEEP\'. This will constrain the aspect ratio to that of the original image.') .'</li>';
  $description .= '<li>'. t('The string \'CROP\'. This will constrain the aspect ratio to the dimensions set above.') .'</li></ul>';
  $description .= t('Leave blank for no aspect ratio constraints.');

  $form['aspect'] = array(
    '#type' => 'textfield',
    '#title' => t('Aspect ratio'),
    '#default_value' => $data['aspect'],
    '#description' => $description,
    '#element_validate' => array('imagecrop_validate_aspect'),
  );
  $form['disable_if_no_data'] = array(
    '#type' => 'checkbox',
    '#title' => t('Don\'t crop if cropping region wasn\'t set.'),
    '#default_value' => $data['disable_if_no_data'],
  );
  return $form;
}

/**
 * Validation of the aspect ratio entry
 */
function imagecrop_validate_aspect(&$element, &$form_state) {

  if (!is_numeric($element['#value'])) {
    if (strtolower($element['#value']) != 'keep' && strtolower($element['#value']) != 'crop') {
      if (!empty($element['#value'])) {
        drupal_set_message(t('Aspect ratio must be a number, the string KEEP, CROP, or empty if you don\'t want to fix it. Defaulting to empty.'));
      }
      form_set_value($element, FALSE, $form_state);
    }
    else {
      form_set_value($element, strtoupper($element['#value']), $form_state);
    }
  }
  else {
    form_set_value($element, abs((float)$element['#value']), $form_state);
  }

}

/**
 * Display properties of a single action
 *
 * @param $element passed on by imagecache
 * @return string
 */
function theme_imagecrop_javascript($element) {
  $data = $element['#value'];
  return 'width: '. $data['width'] .', height: '. $data['height'] .', xoffset: '. $data['xoffset'] .', yoffset: '. $data['yoffset'] .', resizable: '. $data['resizable'] .', aspect ratio: '. $data['aspect'] .', don\'t crop if region is not set: '. $data['disable_if_no_data'];
}

/**
 * Callback to perform the crop on an image
 *
 * @param $image current image resource
 * @param $data values associated with this action
 * @return false or true
 */
function imagecrop_javascript_image(&$image, $data) {

  $presetid = '';
  // if a global presetid is been set, it meens the image is generated from the imagecrop module
  if (isset($GLOBALS['imagecrop_presetid'])) {
    $presetid = $GLOBALS['imagecrop_presetid'];
  }
  // and if not, then get the id from list of all presets
  else {

    $args = explode('/', $_GET['q']);
    $key = array_search('imagecache', $args);
    if ($key != FALSE) {
      $key++;
      $presetname = $args[$key];
      $all_presets = imagecache_presets();
      foreach ($all_presets as $preset) {
        if ($preset['presetname'] == $presetname) {
          $presetid = $preset['presetid'];
          break;
        }
      }

    }

  }

  if (!empty($presetid)) {

    $row = FALSE;
    $hooks = variable_get('imagecrop_modules', array());

    if (in_array('profile_picture', $hooks)) {

      $is_profile_picture = imagecrop_match_requested_file_directory($image->source, variable_get('user_picture_path', 'pictures'));
      if ($is_profile_picture) {
        $row = db_fetch_object(db_query("
          SELECT xoffset, yoffset, width, height, scale
            FROM {imagecrop} ic
            INNER JOIN {users} u on u.uid = ic.fid
          WHERE u.picture = '%s' AND ic.presetid = %d AND reference = 'user'"
          , $image->source, $presetid));
      }

    }

    // support for node images (this sucks in a way, because images are not stored in the files table)
    if (!$row && module_exists('node_images')) {

      $directory = variable_get('node_images_path', 'node_images');
      $pos = strpos($image->source, $directory);
      if ($pos !== FALSE) {
        $row = db_fetch_object(db_query("
          SELECT xoffset, yoffset, width, height, scale
            FROM {imagecrop} ic
            INNER JOIN {node_images} ni on ni.id = ic.fid
          WHERE ni.filepath = '%s' AND ic.presetid = %d AND reference = 'node_images'"
          , $image->source, $presetid));
      }

    }

    if (!$row) {
      $row = db_fetch_object(db_query("
        SELECT xoffset, yoffset, width, height, scale
          FROM {imagecrop} ic
          INNER JOIN {files} f on f.fid = ic.fid
        WHERE f.filepath = '%s' AND ic.presetid = %d AND reference = 'files'"
        , $image->source, $presetid));
    }

    $firstscale = FALSE;
    // fill cropping info from database
    if (!empty($row)) {
      $data['xoffset'] = $row->xoffset;
      $data['yoffset'] = $row->yoffset;
      $data['width'] = $row->width;
      $data['height'] = $row->height;
      $firstscale = TRUE;
    }
    else {
      // If there is no data in DB, use default or exit
      if ($data['disable_if_no_data'] == 1) {
        return TRUE;
      }
    }

    // add scale if necessary
    if ($row->scale != 'original' && $firstscale == TRUE) {
      if (!imageapi_image_scale($image, $row->scale, '', FALSE)) {
        watchdog('imagecrop', 'imagecache_scale_image failed before imagecrop', WATCHDOG_ERROR);
        return FALSE;
      }
    }
  }

  if (!imageapi_image_crop($image, $data['xoffset'], $data['yoffset'], $data['width'], $data['height'])) {
    watchdog('imagecrop', 'imagecrop_javascript failed. image: %image, data: %data.', array('%path' => $image, '%data' => print_r($data, TRUE)), WATCHDOG_ERROR);
    return FALSE;
  }
  return TRUE;
}

/**
 * Check if a the current requested file is located in the given directory.
 */
function imagecrop_match_requested_file_directory($filepath, $search_directory) {

  $files_directory = variable_get('file_directory_path', 'sites/default/files');

  $image_dir = str_replace($files_directory .'/', '', $filepath);
  $args = explode('/', $image_dir);
  unset($args[count($args) - 1]);
  $image_dir = implode('/', $args);

  return $image_dir == $search_directory;

}