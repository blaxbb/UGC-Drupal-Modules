<?php
/**
 * @file
 * UGC Game Module
 */

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function infection_game_register_player($game, $uid, $data = NULL, $form = NULL, $form_state = NULL, $killid = NULL, $oz = NULL) {
  global $user;
  $game_id = $game['game_id'];

  $kill_id_length = $game['gamestate']['killidLength'];

  $data = array();



  if ($form_state != NULL) {
    $oz = $form_state['values']['ozvolunteer'];
  }
  elseif ($oz == NULL) {
    $oz =0;
  }

  if ($killid == NULL) {
    $killid = game_generate_id($kill_id_length);
  }
  //TODO CHECK DUPES

  $player = array(
    'uid' => $uid,
    'ozvolunteer' => $oz,
    'killid' => $killid,
    'data' => serialize($data),
  );

  infection_save_player($game, $player);

  return;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function infection_save_player($game, $player) {
  db_merge('infection_data')
    ->key(array('gameid' => $game['game_id'], 'uid' => $player['uid']))
    ->fields(array(
    'gameid' => $game['game_id'],
    'uid' => $player['uid'],
    'status' => isset($player['status']) ? $player['status'] : 0,
    'killid' => $player['killid'],
    'tags' => isset($player['tags'] ) ? $player['tags'] : 0,
    'killedbyuid' => isset($player['killedbyuid']) ? $player['killedbyuid'] : 0,
    'killedtime' => isset($player['killedtime']) ? $player['killedtime'] : 0,
    'ozvolunteer' => isset($player['ozvolunteer']) ? $player['ozvolunteer'] : 0,
    'data' => serialize($player['data']),
  ))
    ->execute();
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function infection_game_delete_player($game, $uid) {
  db_delete('infection_data')
  ->condition('gameid', $game['game_id'], '=')
  ->condition('uid', $uid, '=')
  ->execute();
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function infection_game_get_player($game_id, &$result) {
  //drupal_set_message('<pre>'.print_r($result,TRUE).'</pre>');
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function infection_game_get_all_players($game) {
  $game_id = $game['game_id'];


  $result = db_select('infection_data', 'g')
    ->fields('g')
    ->condition('gameid', $game_id, '=')
    ->execute();

  $players = array();

  foreach ($result as $res) {

    if (isset($res->data)) {
      $res->data = unserialize($res->data);
    }
    $players[$res->uid] = $res;
  }

  return $players;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function infection_game_get_db_table() {
  return 'infection_data';
}
