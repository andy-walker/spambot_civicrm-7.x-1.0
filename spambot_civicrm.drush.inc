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
                "But they are moved to a group rather than directly deleting, so they can be reviewed by an " . 
                "administrator beforehand, just in case."
        ),
        'options' => array()
    );

    return $items;

}

function drush_spambot_civicrm($command) {
    
    switch ($command) {
        
        case 'move-to-group':
            
            civicrm_initialize();
            $contact_ids = array();

            # query all contacts without a corresponding drupal user
            # (the uf_match record gets deleted when spambot deletes a user)
            $dao = CRM_Core_DAO::executeQuery("
                SELECT c.id FROM civicrm_contact c
             LEFT JOIN civicrm_uf_match ufm ON ufm.contact_id = c.id
                 WHERE ufm.contact_id IS NULL
            ");
            while ($dao->fetch())
                $contact_ids[] = $dao->id;

            # create group
            if ($contact_ids) {

                $result = civicrm_api('group', 'create', array(
                    'version'   => 3,
                    'title'     => dt('Spambot CiviCRM Contacts - @date', array('@date' => date('d/m/Y H:i:s'))),
                    'name'      => dt('spambot_@date', array('@date' => date('YmdHis'))),
                    'is_active' => 1
                ));

                if ($result['is_error']) {
                    
                    drush_log(dt(
                        'Unable to create group. @message', 
                        array('@message' => $result['error_message'])
                    ), 'error');

                } else { 

                    $group = reset($result['values']);

                    drush_log(dt('Created group: @name', array('@name' => $group['title'])), 'ok');

                    # put contacts in group
                    $params = array(
                        'version'  => 3,
                        'group_id' => $group['id']
                    );

                    foreach ($contact_ids as $index => $contact_id)
                        $params['contact_id' . ($index ? ('.' . $index + 1) : '')] = $contact_id;

                    $result = civicrm_api('GroupContact', 'create', $params);
                    
                    if ($result['is_error']) {
                        drush_log(dt(
                            'Unable to add contacts to group. @message', 
                            array('@message' => $result['error_message'])
                        ), 'error');
                    } else {
                        drush_log(dt(
                            '@some suspected spam contacts added to group.', 
                            array('@some' => count($contact_ids))
                        ), 'ok');
                    }

                }

            } else {
                drush_log(dt('No suspected spam contacts found.'), 'ok');
            }

            break;
        
        default:

            drush_log(dt('Unrecognized command: @command', array('@command' => $command)), 'error');
            break;
    
    }

}