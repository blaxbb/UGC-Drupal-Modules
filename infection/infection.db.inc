<?php
/**
 * @file
 * UGC Game Module
 */

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function infection_game_save_player($player) {

  $game = game_get_game($player['gameid']);
  $killidlength = $game['gamestate']['killidLength'];
  $killid = isset($player['killid']) ? $player['killid'] : game_generate_id($killidlength, $game);


  db_merge('infection_data')
    ->key(array('gameid' => $player['gameid'], 'uid' => $player['uid']))
    ->fields(array(
    'gameid' => $player['gameid'],
    'uid' => $player['uid'],
    'status' => isset($player['status']) ? $player['status'] : 0,
    'killid' => $killid,
    'tags' => isset($player['tags'] ) ? $player['tags'] : 0,
    'killedbyuid' => isset($player['killedbyuid']) ? $player['killedbyuid'] : 0,
    'killedtime' => isset($player['killedtime']) ? $player['killedtime'] : 0,
    'ozvolunteer' => isset($player['ozvolunteer']) ? $player['ozvolunteer'] : 0,
    'data' => isset($player['data']) ? serialize($player['data']) : NULL,
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

function infection_get_player_from_killid($game, $killid)
{
  $query = db_select('infection_data', 'i');
  $query->join('users', 'u', 'i.uid = u.uid');
  $query->join('game_data', 'g', 'g.uid = i.uid');
  $query->fields('i');
  $query->fields('g')
    ->fields('u', array('name', 'picture'))
    ->condition('g.gameid', $game['game_id'], '=')
    ->condition('i.killid', $killid, '=');
    

  $result = $query->execute()->fetchAssoc();
  if (isset($result['data'])) {
    $result['data'] = unserialize($result['data']);
  }  
  return $result;
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
