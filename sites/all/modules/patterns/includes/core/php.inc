<?php

// Must be included here and not in the .module file
module_load_include('inc', 'patterns', 'includes/core/common');

/**
 * @file
 * A simple, sequential version of running patterns.
 */

/**
 * Execute a Pattern. Actions will be called sequentially.
 *
 * E.g.: loading additional modules, and creating the array of patterns actions.
 *
 * @param stdClass $pattern Pattern object as loaded by patterns_get_pattern().
 * @param array $params Parameters for executing the pattern. Array format as follows:
 * - pid => Pid of the pattern as it is in the database
 * - run-subpatterns => ['first-update', always', 'update', 'first', 'never']
 * - quickrun => boolean flag to indicate that the execution was initiated from a quickrun
 *   and the pattern is already loaded into $pattern.
 * @param $patterns_details
 * @param $actions_map
 *
 * @return
 * @TODO Doc.
 */
function patterns_execute_pattern_php($pattern, $params, $patterns_details, $actions_map) {
  $pattern_details = current($patterns_details);
  $pid = key($patterns_details);

  $info = $pattern_details['info'];
  $infos = array($pid => $info);

  $modules = $pattern_details['modules'];
  $sections = $pattern_details['sections'];

  $include_options = patterns_parser_build_include_options($pid, $params['run-subpatterns']);

  $global_success = TRUE;
  ///////////////////////////////
  // Looping through the sections
  ///////////////////////////////

  foreach ($sections as $section => $actions) {
    $actions = patterns_parser_retrieve_actions_from_section($actions, $include_options);
    // This can happen if includes are disabled
    if (count($actions) == 0 ) {
      drupal_set_message(t('Section "%section" did not contain any valid action. Please check your policy for included patterns.', array('%section' => $section)), 'warning');
      continue;
    }

    // Reformat the actions. $actions passed as reference.
    // If includes are found, they are exploded into other actions
    patterns_reformat_actions($actions);

    $i = 0;
    $success = TRUE;
    foreach ($actions as $action) {
      $results = patterns_php_action($action['action'], $action['data'], $i, $actions_map);
      
      if ($results['status'] == PATTERNS_ERR) {
        _patterns_section_fail($pid, $info['title'], $section);
        $success = FALSE;
        $global_success = FALSE;
        break;
      }
      $i++;
    }
    if ($success) {
    	_patterns_section_success($pid, $info['title'], $section);
    }
  }
  drupal_flush_all_caches();
 
  return $global_success;
}

/**
 *
 * Execute a single pattern action.
 *
 * @param array $action
 * @param mixed $place index of the current operation within the batch_set
 * @param array $actions_map [$pid pattern id, $index ??]
 * @TODO Doc.
 */
function patterns_php_action($action, $data, $place, $actions_map) {

  patterns_io_load_components(); // TODO: move this out of here?

  // Nothing to do if there is no action
  if (empty($data) || empty($action)) {
    drupal_set_message(t('Cannot execute empty action.'), 'error');
    return FALSE;
  }

  // $actions passed as reference.
  $results = patterns_prepare_action($action, $data);
  if ($results['status'] == PATTERNS_ERR) {
    return $results;
  }

  $identifiers = array();
  return patterns_implement_action($action, $data, $identifiers, $place, $actions_map);
}
