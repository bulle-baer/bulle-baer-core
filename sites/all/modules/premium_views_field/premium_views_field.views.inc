<?php

function premium_views_field_views_data() {
  $data['premium'] = array(
    'table' => array(
      'group' => 'Premium',
      'title' => 'premium_nid',
      'join' => array(
        'node' => array(
          'left_field' => 'nid',
          'field' => 'nid',        
         ),
      ),
    ),
    'nid' => array(
      'title' => t('Premium Status'),
      'help' => t('Node status as defined by Premium module.'), // The help that appears on the UI,
      // Information for displaying the nid
      'field' => array(
        'handler' => 'views_handler_field_node',
        'click sortable' => TRUE,
      ),

    ),
  );

  return $data;  
}