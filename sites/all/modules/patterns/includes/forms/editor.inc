<?php
/**
 * @file
 * Functions, forms related to the Patterns editor.
 */


/**
 * Display the page for creating a new pattern.
 */
function patterns_editor_create_page($pid = NULL) {

  $info = array();
  $pattern = array();

  if (is_numeric($pid)) {
    $pattern = patterns_get_pattern($pid);

    if ($pattern->content) {
      $info['Title'][] = t('Title:');
      $info['Title'][] = $pattern->title;
      $info['Path'][] = t('Path:');
      $info['Path'][] = $pattern->file;
    }
  }
  $out = '<h6>Info</h6>';
  $out .= theme('table', array('rows' => $info, 'attributes' => array('class' => 'patterns-list')));
  $out .= patterns_editor('patterns_edit', $pattern);

  return $out;
}



/**
 * Builds up a pattern editing environment. Loads additional javascript libraries,
 * and supplies AJAX validation on the fly.
 *
 * @param mixed $pattern
 * @return
 * @todo Doc.
  */
function patterns_editor($form_id, $pattern = array()) {

  $pattern = patterns_get_pattern_obj($pattern);

  $form = drupal_get_form($form_id, $pattern);

  if(($form_id == "patterns_quickrun" || $form_id == "patterns_import_source") && !patterns_parser_ready()){
    $messages = t('No available patterns parser was found.</br>');
    $messages .= t(' Go to the !modules page to enable more Patterns parsers.', array('!modules' => l(t('modules'), 'admin/modules')));
    drupal_set_message($messages,'warning');
    return $form;
  }

  $editor  = drupal_render($form);
  if (!patterns_load_codemirror($form['format']['#default_value'])) {
    $cm = l(t('Codemirror 3'), 'http://codemirror.net/');
    $editor .= t('Install !cm in sites/all/libraries/codemirror for a better editor experience.', array('!cm' => $cm));
  }

  // Load the JS of for the ajax validation.
  drupal_add_js(drupal_get_path('module', 'patterns') . '/js/ajax_validation.js');

  return $editor;
}

/**
 * Display the page for editing a pattern.
 */
function patterns_edit_page($pid = NULL) {

  $form = array();

  if (!patterns_parser_ready()) {
    $messages = t('No available patterns parser was found.</br>');
    $messages .= t(' Go to the !modules page to enable more Patterns parsers.', array('!modules' => l(t('modules'), 'admin/modules')));
    drupal_set_message($messages,'warning');
    return $form;
  }

  $pattern = patterns_utils_if_invalid_go_back($pid);

  if (!$pattern->file) {
    drupal_set_message(t('This pattern does not seem to be associated with a valid source file.'), 'error');
    return FALSE;
  }

  $info = array();
  $info['Path'][] = t('Path:');
  $info['Path'][] = $pattern->file;
  $info['Enabled'][] = t('Enabled:');
  $info['Enabled'][] = ($pattern->enabled) ? t('Yes') : t('No');

  $out = '<h6>Info</h6>';
  $out .= theme('table', array('rows' => $info, 'attributes' => array('class' => 'patterns-list')));
  $out .= patterns_editor('patterns_edit', $pattern);

  return $out;
}


/**
 * Form constructor for editing a pattern.
 * TODO:params
 * @see patterns_edit_validate()
 * @see patterns_edit_submit()
 * @ingroup forms
 */
function patterns_edit($form, &$form_state, $pattern, $full = TRUE) {

  if (!patterns_engine_is_on()) {
    $settings = l(t('settings'), 'admin/patterns/settings');
    drupal_set_message(t('Patterns engine is off. You can change the state of the engine from the !settings page if you want to execute the pattern.', array('!settings' => $settings)), 'warning');
  }

  // TODO: this form could be reused, see function patterns_enable_pattern().
  // Default
  $content = '# Pattern';
  $format = (empty($pattern->format)) ? PATTERNS_FORMAT_UNKNOWN
                                      : $pattern->format;
  $title = t('Pattern File');
  $description = '';
  $validation = patterns_db_analyze_patterns_status($pattern->status);

  $form['name'] = array(
    '#type' => 'value',
    '#value' => $pattern->name,
  );

  $form['pid'] = array(
    '#type' => 'value',
    '#value' => $pattern->pid
  );

  $form = patterns_forms_get_formats_selector($form, $format);
  $form = patterns_forms_get_validation_level_selector($form, PATTERNS_VALIDATE_SYNTAX);

  if (!empty($pattern->file)) {
    if (file_exists($pattern->file) && is_readable($pattern->file)) {
      $content = file_get_contents($pattern->file);
    }
    else {
      $content = FALSE;
    }
  }

  if (!$content) {
    drupal_set_message(t('The pattern file was removed from its stored location in the file system. Trying to load from the database.'), 'error');
    if (empty($pattern->pattern)) {
      drupal_set_message(t('No valid pattern found in the database either.', 'ERR'));
    }
    else {
      $content = _patterns_editor_dump_from_db($pattern->pattern, $format);
    }

    $title = t('Pattern file recovered from the database');
    $description = t('Comments are not stored in the database.') . '</br></br>';

  }
  else {
    // Pattern was updated in the file system. Show what was in the database
    if (patterns_db_is_pattern_updated($pattern)) {

      $db_content = _patterns_editor_dump_from_db($pattern->pattern, $format);

      $form['db'] = array(
        '#type' => 'fieldset',
        '#title' => t('The Pattern file stored in the database'),
        '#prefix' => '<strong>' . t('Warning!! A newer version of this pattern was found in the file system, and both the version stored in the database, and the one just found are displayed here for comparison.') . '</strong><br/>',
        '#description' => t('This is the old version of the pattern found in the database. This is what will be executed if this pattern is run.') . '</br></br>',
        '#collapsibale' => TRUE,
      );

      $form['db']['validation_result_db'] = array(
        '#markup' => '<strong>' . $validation . '</strong>',
        '#prefix' => '<div id="validation_result_db">',
        '#suffix' => '</div>',
      );

      $form['db']['content_db'] = array(
        '#type' => 'textarea',
        '#title' => t('Pattern\'s code'),
        '#rows' => 25,
        '#default_value' => $db_content,
        '#disabled' => 'true',
      );


      // Update the validation string for the new pattern in the fs.
      $title = t('Pattern file as loaded from the file system');
      $validation = 'This pattern has not been validated yet.';
      $description = t('Newer version of the pattern found in the file system.') . '</br></br>';
    }
  }

  $form['fs'] = array(
    '#type' => 'fieldset',
    '#title' => $title,
    '#description' => $description,
    '#suffix' => '</br>',
  );

  $form['fs']['validation_result'] = array(
    '#markup' => '<strong>' . $validation . '</strong>',
    '#prefix' => '<div id="validation_result">',
    '#suffix' => '</div>',
  );

  $form['fs']['content'] = array(
    '#type' => 'textarea',
    '#title' => t('Pattern\'s code'),
    '#rows' => 25,
    '#default_value' => $content,
  );

  $form['fs']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['fs']['validate'] = array(
    '#prefix' => '<span style=\'margin-right:10px\'>',
    '#markup' => "<a href='#' id='validate_pattern'>" . t('Validate') . "</a>",
    '#suffix' => '</span> ',
  );

  if (patterns_engine_is_on()) {
    $form['fs']['run'] = array(
      '#prefix' => '<span style=\'margin-right:10px\'>',
      '#markup' => l(t('Run saved pattern'), 'admin/patterns/enable/' . $pattern->pid),
      '#suffix' => '</span> ',
    );
  }

  $form['fs']['cancel'] = array(
    '#markup' => l(t('Back'), 'admin/patterns'),
  );

  return $form;
}

/**
 * Form validation handler for patterns_edit_form().
 *
 * @todo Do this properly.
 *
 * @see patterns_edit_submit()
 */
function patterns_edit_validate($form, &$form_state) {

	// must be real path
  $path = patterns_path_get_files_dir();

  if (!file_prepare_directory($path, FILE_CREATE_DIRECTORY)) {
    form_set_error('form_token', t('Path @path does not exist or is not writable.', array('@path' => $path)));
  }
}

/**
 * Form submission handler for patterns_edit_form().
 *
 * @see patterns_edit_validate()
 */
function patterns_edit_submit($form, &$form_state) {
  $name = $form_state['values']['name'];
  $content = $form_state['values']['content'];
  $format = $form_state['values']['format'];

  patterns_io_save_pattern($content, $name, $format);
}


/**
 * Adds the JavaScript files to load the Co2 Editing textarea.
 */
function patterns_load_codemirror($format = PATTERNS_FORMAT_YAML) {
  // Load the CodeMirror Editor if installed in the libraries folder
  $library = libraries_load('codemirror');
  if (!empty($library['loaded'])) {
    // Adding Pattern Customization for Co2
    drupal_add_css(drupal_get_path('module', 'patterns') . '/css/editor.css');
    drupal_add_js(drupal_get_path('module', 'patterns') . '/js/editor.js');
    return TRUE;
  }
  return FALSE;
}

/**
 * Helper function which returns the pattern dumped from the database, or
 * an error string.
 *
 * @param array $pattern The pattern to dump.
 * @param mixed $format The format of the pattern array.
 *
 * @return mixed $content A string representing the pattern or an error.
 */
function _patterns_editor_dump_from_db($pattern, $format) {
  $content = patterns_parser_dump($pattern, $format);
  if (!$content) {
    $content = t('An error occurred while dumping the pattern in the database here.');
  }
  return $content;
}
