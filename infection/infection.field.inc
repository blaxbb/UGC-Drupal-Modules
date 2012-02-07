<?php
/**
 * @file
 * UGC Game Module
 */

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function infection_game_widget_form_elements($gametype, $item) {

  if ($gametype['id'] != 'infection') {
    return;
  }
  $form = array();

  $form['killidLength'] = array(
    '#type' => 'textfield',
    '#title' => 'KillID Length',
    '#default_value' => isset($item['gamestate']['killidLength']) ? $item['gamestate']['killidLength'] : 6,
  );
  $form['playerStates'] = array(
    '#type' => 'textfield',
    '#title' => 'Player States',
    '#description' => 'Comma sperated team list',
    '#default_value' => isset($item['gamestate']['playerStates'])
        ? implode(",", $item['gamestate']['playerStates']) : 'Human, Zombie, OZ (Hidden), OZ (Revealed)',
  );
  return $form;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function infection_game_field_update($gamestate) {
  drupal_set_message('1<pre>' . print_r($gamestate, TRUE) . '</pre>');
  if (!is_array($gamestate['playerStates'])) {
    $gamestate['playerStates'] = explode(',', $gamestate['playerStates']);
  }

  drupal_set_message('2<pre> ' . print_r($gamestate, TRUE) . '</pre>');
  return $gamestate;
}
