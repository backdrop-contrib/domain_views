<?php

/**
 * @file
 *  Domain Views plugin that caches views on a per domain basis.
 *  This is necessary for views that filter on "current domain"
 *  (ex. SELECT * FROM {node} WHERE domain_source = current_domain)
 *  otherwise "current domain" will be cached.
 *
 * Code by bleen16
 * @link http://drupal.org/user/77375
 * @link http://drupal.org/node/782208
 */

/**
 * Cache plugin that provides caching on a per domain basis.
 */
class domain_views_plugin_cache_time extends views_plugin_cache_time {
  function summary_title() {
    // Return a sumary title for the views admin screen (ex. 1 min/1min (Per Domain)).
    return format_interval($this->options['results_lifespan'], 1) . '/' . format_interval($this->options['output_lifespan'], 1) . ' (Per Domain)';
  }

  function get_results_key() {
    /**
     * Create an md5 hashed key including the current domain
     * to use as the cache key for caching the views results.
     */
    if (!isset($this->_results_key)) {
      // Set the results key.
      $this->_results_key = $this->view->name . ':' . $this->display->id . ':results:' . $this->get_cache_key();
    }

    return $this->_results_key;
  }

  function get_output_key() {
    /**
     * Create an md5 hashed key including the current domain
     * to use as the cache key for caching the views output.
     */
    if (!isset($this->_output_key)) {
      $key_data = array(
        'theme' => $GLOBALS['theme'],
      );
      // Match the results key.
      $this->_output_key = $this->view->name . ':' . $this->display->id . ':output:' . $this->get_cache_key($key_data);
    }

    return $this->_output_key;
  }

  /**
   * Returns cache key.
   * @see  views_plugin_cache::get_cache_key()
   *
   * @param array $key_data
   *   Additional data for cache segmentation and/or overrides for default
   *   segmentation.
   *
   * @return string
   */
  function get_cache_key($key_data = array()) {
    global $user;
    global $_domain;

    $key_data += array(
      'roles' => array_keys($user->roles),
      'super-user' => $user->uid == 1, // special caching for super user.
      'language' => $GLOBALS['language'],
      'domain' => $_domain['domain_id'], // Adding current domain to key data.
    );

    if (empty($key_data['build_info'])) {
      $build_info = $this->view->build_info;
      foreach (array('query','count_query') as $index) {
        // If the default query back-end is used generate SQL query strings from
        // the query objects.
        if ($build_info[$index] instanceof SelectQueryInterface) {
          $query = clone $build_info[$index];
          $query->preExecute();
          $key_data['build_info'][$index] = array(
            'sql' => (string) $query,
            'arguments' => $query->getArguments(),
          );
        }
      }
    }
    $key = md5(serialize($key_data));
    return $key;
  }
}
