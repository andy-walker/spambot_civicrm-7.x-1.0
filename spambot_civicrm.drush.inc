<?php

/**
 * Implementation of hook_drush_command()
 */
function spambot_civicrm_drush_command() {
    
    $items['spambot-civicrm'] = array(
        'description' => 'Drush command included with spambot_civicrm module',
        'examples'    => array(
            'drush spambot-civicrm move-to-group' => 
                "Move contacts without a corresponding Drupal user to a Civi group which will be created. " . 
                "This is so you can potentially delete any contacts where spambot has already deleted the user. " . 
                "But they are moved to a group, so they can be reviewed by an administrator before deleting, just in case."
        ),
        'options' => array()
    );

    return $items;

}

function drush_spambot_civicrm($command) {
    
    switch ($command) {
        
        case 'move-to-group':
            # todo: query all contacts without a corresponding drupal user
            # todo: create group
            # todo: put contacts in group
            break;
        
        default:
            drush_log(dt('Unrecognized command: @command', array('@command' => $command)), 'error');
            break;
    
    }

}