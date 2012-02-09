<?php
/**
 * @file
 * UGC Game Module
 */

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function infection_form_game_register_form_alter(&$form, &$form_state, $form_id) {

  for($i = 2; $i < 60; $i++)
  {
    $player = array('uid' => $i, 'gameid' =>$form['#game']['game_id']);
    game_save_player($player);
  }

  if ($form['#game']['type_id'] != 'infection') {
    return;
  }
  global $user;

  if ($form['#registered']) {
    $player = game_get_player($form['#game']['game_id'], $user->uid);

    $form['ozvolunteer'] = array(
      '#type' => 'checkbox',
      '#title' => 'OZ Volunteer?',
      '#default_value' => isset($player['ozvolunteer']) ? $player['ozvolunteer'] : FALSE,
    );
  }

  else {
    $form['ozvolunteer'] = array(
      '#type' => 'checkbox',
      '#title' => 'OZ Volunteer?',
      '#default_value' => FALSE,
    );
    $form['#validate'][] = 'infection_register_form_validate';
        
  }
}

function infection_register_form_validate($form, &$form_state) {

  $player = $form_state['values']['#player'];
  $game = $form['#game'];

  $kill_id_length = $game['gamestate']['killidLength'];

  $data = array();



  if ($form_state != NULL) {
    $oz = $form_state['values']['ozvolunteer'];
  }
  elseif ($oz == NULL) {
    $oz =0;
  }
  
  $killid = isset($player['killid']) ? $player['killid'] : NULL;
  
  if ($killid == NULL) {
    $killid = game_generate_id($kill_id_length, $game);
  }
  
  if($killid == FALSE)
    form_set_error('', 'Failed to generate an ID.  Either you are very (un)lucky, or something went wrong');
  
  //TODO CHECK DUPES
  $player['ozvolunteer'] = $oz;
  $player['killid'] = $killid;
  $player['data'] = serialize($data);

  $form_state['values']['#player'] = $player;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function infection_form_game_player_edit_form_alter(&$form, &$form_state, $form_id) {

  $game = $form['#game'];
  $player = $form['#player'];

  $form['status'] = array(
    '#type' => 'radios',
    '#title' => t('Status'),
    '#default_value' => isset($player['status']) ? $player['status'] : 0,
    '#options' => $game['gamestate']['playerStates'],
    '#description' => t('Change the players ingame status.'),
  );

  $form['killid'] = array(
    '#type' => 'textfield',
    '#title' => t('Kill ID'),
    '#description' => t('Leave blank to generate new Kill ID'),
    '#default_value' => isset($player['killid']) ? $player['killid'] : '',
    '#size' => $game['gamestate']['killidLength'],
    '#maxlength' => $game['gamestate']['killidLength'],
  );
  $form['tags'] = array(
    '#type' => 'textfield',
    '#title' => t('Tags'),
    '#default_value' => isset($player['tags']) ? $player['tags'] : 0,
    '#size' => 3,
    '#maxlength' => 6,
    '#required' => TRUE,
  );

  $form['killedby'] = array(
    '#type' => 'textfield',
    '#title' => t('Killed by'),
    '#maxlength' => 60,
    //'#autocomplete_path' => 'user/autocomplete',
    '#default_value' => isset($player['killedbyuid']) ? $player['killedbyuid'] : 0,
  );

  //TODO DATE API INTEGRATION

  $form['killedtime'] = array(
    '#type' => 'textfield',
    '#title' => t('Time of Death'),
    '#default_value' => isset($player['killedtime']) ? $player['killedtime'] : 0,
    '#size' => 60,
    '#maxlength' => 60,
    '#required' => TRUE,
  );

  $form['ozvolunteer'] = array(
    '#type' => 'checkbox',
    '#title' => 'OZ Volunteer',
    '#default_value' => isset($player['ozvolunteer']) ? $player['ozvolunteer'] : FALSE,
  );

  $form['#validate'][] = 'infection_edit_validate';
  $form['#submit'][] = 'infection_edit_submit';

}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function infection_edit_validate(&$form, &$form_state) {
  $game = $form['#game'];

  if ($form_state['values']['killid'] == '') {
    $form_state['values']['killid'] = game_generate_id($game['gamestate']['killidLength']);
  }
  //TODO CHECK DUPES
  if (strlen($form_state['values']['killid']) != $game['gamestate']['killidLength']) {
    form_set_error('killid', t('Improper killID length'));
  }

  if (!is_numeric($form_state['values']['tags'])) {
    form_set_error('tags', t('Tags must be numeric'));
  }

  if (!is_numeric($form_state['values']['killedtime'])) {
    form_set_error('killedtime', t('Killed time must be numeric'));
  }

  if (!db_query("SELECT COUNT(*) FROM {users} WHERE uid = :uid", array(':uid' => $form_state['values']['killedby']))->fetchField()) {
    // User exists
    form_set_error('killedby', t("Killer doesn't exist"));
  }

}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function infection_edit_submit(&$form, &$form_state) {
  drupal_set_message("GOT SUBMIT");
  $player = $form['#player'];
  $player['status'] = $form_state['values']['status'];
  $player['killid'] = $form_state['values']['killid'];
  $player['tags'] = $form_state['values']['tags'];
  $player['killedbyuid'] = $form_state['values']['killedby'];
  $player['killedtime'] = $form_state['values']['killedtime'];
  $player['ozvolunteer'] = $form_state['values']['ozvolunteer'];

  infection_save_player($form['#game'], $player);
  //$player
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function infection_players_page_headers($game) {

  $headers = array(
    array(
      'data' => 'Status',
      'field' => 'status',
      'sort' => 'asc',
    ),
    array(
      'data' => 'Tags',
      'field' => 'tags',
      'sort' => 'asc',
    ),
    array(
      'data' => 'Killed By',
      'field' => 'killedbyuid',
      'sort' => 'asc',
    ),
    array(
      'data' => 'Time of Death',
      'field' => 'killedtime',
      'sort' => 'asc',
    ),
    array(
      'data' => 'OZ',
      'field' => 'ozvolunteer',
      'sort' => 'asc',
    ),
  );

  if (user_access('view other killids')) {
    $headers[] = array(
      'data' => 'Kill ID',
      'field' => 'killid',
      'sort' => 'asc',
    );
  }

  return $headers;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function infection_players_page_data($game, $player) {

  $time = $player->killedtime;
  if ($time == 0) {
    $date = '';
  }
  else {
    $date = date("F j, Y, g:i a", $time);
  }

  $killer = game_get_player($game, $player->killedbyuid);

  $data = array(
    infection_get_status_string($game, $player->status),
    $player->tags,
    ($killer['status'] == 3) ? "An " . $game['gamestate']['playerStates'][3] : $killer['name'], //TODO OZ SECURITY
    $date,
    $player->ozvolunteer,
  );

  if (user_access('view other killids')) {
    $data[] = $player->killid;
  }

  return $data;
}
