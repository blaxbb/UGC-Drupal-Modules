<?php

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_field_info() {
  return array(
    'game' => array(
      'label' => t('UGC Game'),
      'description' => t('This field stores UGC game data'),
      'default_widget' => 'game',
      'default_formatter' => 'game',
    ),
  );
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_field_validate($entity_type, $entity, $field, $instance, $langcode, $items, &$errors) { }

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_field_is_empty($item, $field) {


  if ($item['game']['games']['selected']) {
    return FALSE;
  }
  return TRUE;

}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_field_formatter_info() {
  return array(
    'game' => array(
      'label' => t('Simple text-based formatter'),
      'field types' => array('game'),
    ),
  );
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();

  switch ($display['type']) {
    // This formatter simply outputs the field as text and with a color.
    case 'game':
      foreach ($items as $delta => $item) {
        $gametype = game_get_gametype($item['type_id']);
        //drupal_set_message('<pre>'.print_r($gametype,TRUE).'</pre>');
        $element[$delta] = array(
          // We create a render array to produce the desired markup,
          // "<p style="color: #hexcolor">The color code ... #hexcolor</p>".
          // See theme_html_tag().
          '#type' => 'html_tag',
          '#tag' => 'p',
          '#value' => t('<pre>' . print_r($item, TRUE) . '</pre>'),
        );
      }
      break;
  }

  return $element;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_field_widget_info() {
  return array(
    'game' => array(
      'label' => t('Game Input'),
      'field types' => array('game'),
    ),
  );
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  game_update_gametype_cache();

  $widget = $element;
  $widget['#delta'] = $delta;
  switch ($instance['widget']['type']) {
    case 'game':
      $widget['games'] = array(
        '#type' => 'fieldset',
        '#title' => 'Gametypes',
        // #delta is set so that the validation function will be able
        // to access external value information which otherwise would be
        // unavailable.
        '#delta' => $delta,
        '#element_validate' => array('game_widget_validate'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );

      $gametypes = game_get_all_gametypes();
      if (!count($gametypes)) {
        $widget['games']['none'] = array(
          '#type' => 'item',
          '#title' => 'No gametypes installed',
        );
      }
      $options = array();

      foreach ( $gametypes as $gametype) {
        $fieldset = array(
          '#type' => 'fieldset',
          '#delta' => $delta,
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
          '#title' => filter_xss($gametype['title']),
        );
        $options[$gametype['id']] = $gametype['title'];

        //TODO switch from module_invoke_all to module_invoke
        $widget['games'][$gametype['id']] = array_merge($fieldset, module_invoke_all('game_widget_form_elements', $gametype, $items[$delta]));
      }

      $arr = array_reverse($options, TRUE);
      $arr[0] = 'None';
      $options = array_reverse($arr, TRUE);
      $widget['games']['selected'] = array(
        '#weight' => -10,
        '#type' => 'select',
        '#title' => t('Select a gametype.'),
        '#options' => $options,
        '#default_value' => $items[$delta]['type_id'],
      );

      break;
  }

  $element['game'] = $widget;
  return $element;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_widget_validate($element, &$form_state) {
  $delta = $element['#delta']; // TODO: Isn't there a better way to find out which element?
  $values = $form_state['values']['field_game']['und'][$delta]['game']['games'];

  //form_set_value($element,
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_field_insert($entity_type, $entity, $field, $instance, $langcode, &$items) {
  for ($i = 0; $i < count($entity->field_game['und']); $i++) {
    $items[$i]['type_id'] = $entity->field_game['und'][$i]['game']['games']['selected'];

    $module = game_get_module_from_gametype($items[$i]['type_id']);

    $items[$i]['gamestate'] = $entity->field_game['und'][$i]['game']['games'][$items[$i]['type_id']];
    $items[$i]['gamestate'] = module_invoke($module, 'game_field_update', $items[$i]['gamestate']);
    $items[$i]['gamestate']['state'] = 0;
    $items[$i]['gamestate'] = serialize($items[$i]['gamestate']);
  }
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_field_update($entity_type, $entity, $field, $instance, $langcode, &$items) {
  drupal_set_message("UPDATING");
  for ($i = 0; $i < count($entity->field_game['und']); $i++) {

    $field_game = $entity->field_game['und'][$i];

    if (isset($field_game['game']['games']['selected'])) {
      //UPDATING FROM FORM
      $items[$i]['type_id'] = $field_game['game']['games']['selected'];

      $items[$i]['gamestate'] = $field_game['game']['games'][$items[$i]['type_id']];
      $items[$i]['gamestate']['state'] = 0;
    }
    else {
      //UPDATING PROGRAMMATICALLY
      $items[$i]['type_id'] = $field_game['type_id'];

      $items[$i]['gamestate'] = $field_game['gamestate'];
      $items[$i]['gamestate']['state'] = $field_game['gamestate']['state'];
    }
    $module = game_get_module_from_gametype($items[$i]['type_id']);

    $items[$i]['gamestate'] = module_invoke($module, 'game_field_update', $items[$i]['gamestate']);
    $items[$i]['gamestate'] = serialize($items[$i]['gamestate']);

  }
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_field_load($entity_type, $entities, $field, $instances, $langcode, &$items, $age) {
  foreach ($entities as $id => $entity) {
    foreach ($items[$id] as $delta => $item) {

      $items[$id][$delta]['gamestate'] = unserialize($items[$id][$delta]['gamestate']);
      $items[$id][$delta]['game_id'] = $id;
      game_invoke($items[$id][$delta], 'game_field_load', $items[$id][$delta]);
    }
  }
}
