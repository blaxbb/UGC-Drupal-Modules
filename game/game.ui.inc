<?php

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_menu() {
  $items['node/%node/register'] = array(
    'title' => 'Register',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('game_register_form', 1),
    'access callback' => 'game_menu_access_node',
    'access arguments' => array(1),
    'type' => MENU_LOCAL_TASK,
  );
  $items['node/%node/settings'] = array(
    'title' => 'Settings',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('game_settings_form', 1),
    'access callback' => 'game_menu_access_settings',
    'access arguments' => array(1),
    'type' => MENU_LOCAL_TASK,
  );
  $items['node/%node/players'] = array(
    'title' => 'Players',
    'page callback' => 'game_get_players_page',
    'page arguments' => array(1),
    'access callback' => 'game_menu_access_node',
    'access arguments' => array(1),
    'type' => MENU_LOCAL_TASK,
  );
  $items['user/%user/games'] = array(
    'title' => 'Games',
    'page callback' => 'game_get_player_list_page',
    'page arguments' => array(1),
    'access callback' => 'game_menu_access_user',
    'access arguments' => array(1),
    'type' => MENU_LOCAL_TASK,
  );
  $items['user/%user/games/%'] = array(
    'title' => 'Games',
    'page callback' => 'game_get_player_info_page',
    'page arguments' => array(1, 3),
    'access callback' => 'game_menu_access_user',
    'access arguments' => array(1, 3),
    'type' => MENU_CALLBACK,
  );
  $items['user/%user/games/%/edit'] = array(
    'title' => 'Game Info',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('game_player_edit_form', 1, 3),
    'access callback' => 'game_menu_access_user_edit',
    'access arguments' => array(1, 3),
    'type' => MENU_NORMAL_ITEM,
  );

  return $items;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_register_form($form, &$form_state, $node, $player = NULL) {
  global $user;

  if ($node->field_game == NULL) {
    return;
  }

  $form['#game'] = $node->field_game['und'][0];
  if (!game_player_is_registered($form['#game'], $user->uid)) {
    $form['#registered'] = FALSE;
    $form['submit'] = array(
      '#type' => 'submit',
      '#weight' => 100,
      '#value' => t('Play ' . $node->title),
    );
  }
  else {
    $form['#registered'] = TRUE;
    $form['submit'] = array(
      '#type' => 'submit',
      '#disabled' => TRUE,
      '#weight' => 100,
      '#value' => t('You are already registered for ' . $node->title),
    );
    $form['quit'] = array(
      '#type' => 'submit',
      '#submit' => array('game_register_form_quit'),
      '#weight' => 100,
      '#value' => t('Quit ' . $node->title),
    );
  }
  return $form;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_register_form_submit($form, &$form_state) {
  global $user;

  $game_id = $form['#game']['game_id'];
  $game_type = $form['#game']['type_id'];
  game_register_player($form['#game'], $user->uid, $form, $form_state);
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_register_form_quit($form, &$form_state) {
  global $user;

  game_delete_player($form['#game'], $user->uid);
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_get_players_page($node) {

  $header = array(
    array(
      'data' => 'Name',
      'field' => 'name',
      'sort' => 'asc',
    ),
  );

  //TODO REPLACE MODULE NAME
  $game = $node->field_game['und'][0];

  $module = game_get_module_from_gametype($game['type_id']);
  $header = array_merge($header, (array) module_invoke($module, 'players_page_headers', $game));



  $players = game_get_all_players_pager($game, $header);

  $rows = array();
  foreach ($players as $player) {

    $data = array(
      'data' => array(
        l($player->name, 'user/' . $player->uid . '/games/' . $game['game_id']),
      ),
    );
    $data = array_merge($data['data'], (array) module_invoke($module, 'players_page_data', $game, $player));

    $rows[] = $data;
  }

  $output = theme('table', array(
      'header' => $header,
      'rows' => $rows,
      'empty' => 'No Players',
    ));

  $output .= theme('pager', array('tags' => array(
      'tags' => array(),
    )));

  return ($output);

}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_get_player_list_page($user) {
  $headers = array('Title');

  $can_edit = user_access('edit players');
  if ($can_edit) {
    $headers[] = 'Edit';
  }
  $items = array();

  foreach (game_get_all_games_for_player($user->uid) as $game) {
    $item = array(
      l(node_load($game->gameid)->title, 'user/' . $user->uid . '/games/' . $game->gameid),
    );
    if ($can_edit) {
      $item[] = l(t('Edit'), 'user/' . $user->uid . '/games/' . $game->gameid . '/edit');
    }
    $items[] = $item;
  }
  $output = theme('table', array('header' => array(
      'header' => $headers,
      'rows' => $items,
      'empty' => 'No Games',
    )));
  return $output;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_get_player_info_page($user, $gameid) {
  return node_load($gameid)->title . ' ' . l(t('Edit'), 'user/' . $user->uid . '/games/' . $gameid . '/edit');
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_player_edit_form($form, &$form_state, $user, $gameid) {
  $form['#game'] = game_get_game($gameid);


  $player = game_get_player($gameid, $user->uid);
  $new_player = FALSE;
  if ($player == NULL) {
    $player['uid'] = $user->uid;
    $player['name'] = $user->name;
    $new_player = TRUE;
  }

  $form['#player'] = $player;



  $form['submit'] = array(
    '#type' => 'submit',
    '#weight' => 100,
    '#value' => $new_player ? t('Register ' . $user->name) : t('Save ' . $user->name),
  );
  if (!$new_player) {
    $form['delete'] = array(
      '#type' => 'submit',
      '#weight' => 100,
      '#value' => t('Delete ' . $user->name),
      '#submit' => array('game_player_edit_form_delete_submit'),
    );
  }

  return $form;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_player_edit_form_submit($form, &$form_state) {
  $game = $form['#game'];
  $player = $form['#player'];
  drupal_set_message('<pre>' . print_r($player, TRUE) . '</pre>');
  if (!game_player_is_registered($game, $player['uid'])) {
    game_register_player($game, $player['uid']);
  }
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_player_edit_form_delete_submit($form, &$form_state) {
  $game = $form['#game'];
  $player = $form['#player'];
  drupal_set_message('kill him');

  game_delete_player($game, $player['uid']);
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_settings_form($form, &$form_state, $node) {
  if (!isset($node->field_game)) {
    return;
  }

  $game = game_get_game($node->nid);
  //drupal_set_message('GAME<pre>'.print_r($game,TRUE).'</pre>');
  $gametype = game_get_gametype($game['type_id']);
  //drupal_set_message('GAMETYPE<pre>'.print_r($gametype,TRUE).'</pre>');
  $states = $gametype->data['states'];

  $form = array();

  $form['#game'] = $game;
  //drupal_set_message('GAME<pre>'.print_r($game,TRUE).'</pre>');
  $form['state'] = array(
    '#weight' => -10,
    '#type' => 'select',
    '#title' => t('Select a gametype.'),
    '#options' => $states,
    '#default_value' => $game['gamestate']['state'],
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#weight' => 100,
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function game_settings_form_submit($form, &$form_state) {
  $game = $form['#game'];
  $node = node_load($game['game_id']);
  //drupal_set_message('F<pre>'.print_r($node->field_game['und'][0]['gamestate'],TRUE).'</pre>');
  $node->field_game['und'][0]['gamestate']['state'] = $form_state['values']['state'];
  //drupal_set_message('form<pre>'.print_r($form_state,TRUE).'</pre>');
  //drupal_set_message('F<pre>'.print_r($node->field_game,TRUE).'</pre>');
  node_save($node);

}
