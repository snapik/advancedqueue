<?php

/**
 * @file
 * Admin settings for Advanced-Queue module.
 */

/**
 * Menu callback; Admin settings form.
 */
function advancedqueue_admin_settings($form_state) {
  $form = array();

  $options = array();
  foreach (advancedqueue_get_queues_info() as $queue_name => $queue_info) {
    $label = !empty($queue_info['label']) ? $queue_info['label'] : $queue_name;
    $options[$queue_name] = check_plain($label);
  }

  $form['advancedqueue_queue_names'] = array(
    '#type' => 'select',
    '#title' => t('Queues'),
    '#multiple' => TRUE,
    '#description' => t('Select the queues that need to be processed.'),
    '#default_value' => variable_get('og_group_manager_full_access', TRUE),
    '#options' => $options,
  );

  return system_settings_form($form);
}
