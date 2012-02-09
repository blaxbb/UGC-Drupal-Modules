<?php


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_save_player($player) {

  $game = game_get_game($player['gameid']);
  db_merge('game_data')
    ->key(array('gameid' => $player['gameid'], 'uid' => $player['uid']))
    ->fields(array(
    'gameid' => $player['gameid'],
    'uid' => $player['uid'],
    'data' => isset($player['data']) ? serialize($player['data']) : NULL,
  ))
    ->execute();

  game_invoke($game, 'game_save_player', $player);
  //module_invoke(game_get_module_from_gametype($game_type), 'game_register_player',$game, $uid, $data, $form, $form_state);
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_delete_player($game, $uid) {

  $game_id = $game['game_id'];
  $game_type = $game['type_id'];

  db_delete('game_data')
    ->condition('gameid', $game_id, '=')
    ->condition('uid', $uid, '=')
    ->execute();
  game_invoke($game, 'game_delete_player', $game, $uid);
  //module_invoke(game_get_module_from_gametype($game_type), 'game_delete_player', $game_id, $uid);
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_get_player($game, $uid) {
  if (!is_array($game)) {
    $fields = field_get_items('node', node_load($game), 'field_game');
    $game = $fields[0];
  }

  $game_id = $game['game_id'];
  $game_type = $game['type_id'];

  $table = module_invoke(game_get_module_from_gametype($game_type), 'game_get_db_table');

  $query = db_select('game_data', 'g');
  $query->join('users', 'u', 'g.uid = u.uid');
  if ($table != "") {
    $query->join($table, 't', 't.uid = g.uid');
    $query->fields('t');
  }

  $query->fields('g')
    ->fields('u', array('name', 'picture'))
    ->condition('g.gameid', $game_id, '=')
    ->condition('u.uid', $uid, '=');

  $result = $query->execute()->fetchAssoc();

  if (isset($result['data'])) {
    $result['data'] = unserialize($result['data']);
  }

  //FIXME
  //drupal_set_message('GT<pre>'.print_r($game_id,TRUE).'</pre>');
  //module_invoke(game_get_module_from_gametype($game_id), 'game_get_player', $game, $result);
  return $result;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_insert_game($game, $data) {
  // TODO Is this unused?

  $game_id = $game['game_id'];
  db_merge('game_data')
    ->key(array('gameid' => $game_id))
    ->fields(array(
    'gameid' => $game_id,
    'data' => serialize($data),
  ))
    ->execute();
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_player_is_registered($game, $uid) {
  if (game_get_player($game, $uid) != NULL) {
    return TRUE;
  }

  return FALSE;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_get_all_games_for_player($uid) {
  $result = db_select('game_data', 'g')
    ->fields('g')
    ->condition('uid', $uid, '=')
    ->execute();

  $games = array();

  foreach ($result as $res) {

    if (isset($res->data)) {
      $res->data = unserialize($res->data);
    }
    $games[$res->gameid] = $res;
  }
  return $games;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_get_all_players($game) {

  $game_id = $game['game_id'];
  $game_type = $game['type_id'];
  //$game_type = game_get_gametype($game_id);
  //FIXME
  $players_from_mod = module_invoke(game_get_module_from_gametype($game_type), 'game_get_all_players', $game);

  $result = db_select('game_data', 'g')
    ->fields('g')
    ->condition('gameid', $game_id, '=')
    ->execute();

  $players = array();

  foreach ($result as $res) {

    if (isset($res->data)) {
      $res->data = unserialize($res->data);
    }

    $merge = array_merge((array) $res, (array) $players_from_mod[$res->uid]);
    $players[$res->uid] = (object) $merge;
  }
  return $players;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_get_all_players_pager($game, $header) {

  $game_id = $game['game_id'];
  $game_type = $game['type_id'];

  $table = module_invoke(game_get_module_from_gametype($game_type), 'game_get_db_table');

  $query = db_select('game_data', 'g')

    ->extend('PagerDefault')
    ->limit(25)
    ->extend('TableSort')
      ->orderByHeader($header);
  $query->join('users', 'u', 'g.uid = u.uid');
  if ($table != "") {
    $query->join($table, 't', 't.uid = u.uid');
  }
  $query->condition('g.gameid', $game_id, '=')    
  ->fields('u', array('name', 'picture'))
  ->fields('g');
  if ($table != "") {
    $query->fields('t');
  }
  $result = $query->execute();

  $players = array();
  foreach ($result as $res) {
    $player = (array) $res;
    if (isset($res->entry['data'])) {
      $player['data'] = unserialize($player['data']);
    }
    if($player['gameid'] == $game_id)
      $players[$res->entry] = (object) $player;
  }
  return $players;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_get_all_games() {
  $result = db_select('game_games', 'g')
    ->fields('g')
    ->execute();
  $results = array();
  foreach ($result as $res) {
    $results[$res->gameid] = unserialize($res->data);
    $results[$res->gameid]['game_id'] = $res->gameid;
  }

  return $results;
}

function _game_create_gametype($data) {
  $title = $data['title'];
  $id = $data['id'];
  $description = isset($data['description']) ? $data['description'] : '';
  $module = $data['module'];
  unset($data['title'], $data['id'], $data['description'], $data['module']);

  db_merge('game_gametypes')
    ->key(array('id' => $id))
    ->fields(array(
    'title' => $title,
    'description' => $description,
    'module' => $module,
    'data' => serialize($data),
  ))
    ->execute();
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_get_module_from_gametype($type_id) {

  $type = game_get_gametype($type_id);

  return $type->module;

}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_get_all_gametypes() {
  $result = db_select('game_gametypes', 'g')
    ->fields('g')
    ->execute();
  $results = array();
  foreach ($result as $res) {
    $results[$res->id] = (array) $res;
    $results[$res->id]['data'] = unserialize($res->data);
  }
  return $results;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_get_gametype($id) {

  $result = db_select('game_gametypes', 'g')
    ->fields('g')
    ->condition('id', $id, '=')
    ->execute();
  $results = array();
  foreach ($result as $res) {
    $results = $res;

    $results->data = unserialize($results->data);
  }

  return $results;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_update_gametype_cache() {


  $del = db_truncate('game_gametypes')
    ->execute();

  $hook = 'game_gametype';
  foreach (module_implements($hook) as $module) {
    $function = $module . '_' . $hook;
    if (function_exists($function)) {
      $result = call_user_func_array($function, array());
      $result['module'] = $module;
      _game_create_gametype($result);
    }
  }
}
