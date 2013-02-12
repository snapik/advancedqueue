<?php

/**
 * @file
 * Drush worker for Advanced-queue.
 */

/**
 * Implements hook_drush_command().
 */
function advancedqueue_drush_command() {
  $items = array();
  $items['advancedqueue-process-queue'] = array(
    'description' => 'Run a processing job for a queue.',
    'arguments' => array(
      'queue' => dt('The name of the queue to process.'),
    ),
    'options' => array(
      'timeout' => 'The maximum execution time of the script. Be warned that this is a rough estimate as the time is only checked between two items.',
      'all' => 'Process all queues.',
    ),
    'aliases' => array('advancedqueue'),
  );
  return $items;
}

function drush_advancedqueue_process_queue($queue = NULL) {
  // Load information about the registred queues, and sort them by weight.
  if (!$queues_info = advancedqueue_get_queues_info()) {
    return drush_set_error(dt('No queues exist.'));
  }

  $all_option = drush_get_option('all');
  if (!$all_option && empty($queue)) {
    return drush_set_error(dt('You have to specify either a set of queues or the --all parameter.'));
  }

  if ($all_option) {
    $queues = $queues_info;
  }
  else {
    // Validate queues.
    $queues = drupal_map_assoc(explode(',', $queue));
    if ($invalid_queues = array_diff_key($queues, $queues_info)) {
      return drush_set_error(dt('The following queues are invalid: !queues. Aborting.', array('!queues' => implode(', ', $invalid_queues))));
    }
    $queues = array_intersect_key($queues_info, $queues);
  }

  // Run the worker for a certain period of time before killing it.
  $timeout = drush_get_option('timeout');
  $end = $timeout ? time() + $timeout : 0;

  drush_log(dt('Starting processing loop.'));

  while (!$end || time() < $end) {
    foreach ($queues as $queue_name => $queue_info) {
      // Add default values.
      $queue_info += array(
        'delete when completed' => TRUE,
      );

      $queue = DrupalQueue::get($queue_name);

      if ($item = $queue->claimItem()) {
        drush_advancedqueue_process_item($queue, $queue_name, $queue_info, $item, $end);
        continue 2;
      }
    }

    // No item processed in that round, let the CPU rest.
    sleep(1);
  }

  drush_log(dt('Timeout: exiting processing loop.'));
}
