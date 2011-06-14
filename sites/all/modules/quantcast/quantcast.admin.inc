<?php

// $Id: quantcast.admin.inc,v 1.1 2009/07/07 04:34:09 inadarei Exp $

/**
 * @file
 * Admin callbacks for the Quantcast module.
 */

/**
 * Implementation of hook_admin_settings()
 */
function quantcast_admin_settings_form(&$form_state) {

  $form['quantcast_tag'] = array(
    '#type' => 'textarea',
    '#title' => t('Quantcast Tag'),
    '#default_value' => variable_get('quantcast_tag', ''),
    '#size' => 20,
    '#rows' => 7,
    '#required' => TRUE,
    '#description' => t('If you do not already have a Quantcast Tag, please acquire it by registering at: !link', array('!link'=>l('http://www.quantcast.com/user/signup','http://www.quantcast.com/user/signup'))),
  );

  return system_settings_form($form);
}
