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
            
            civicrm_intialize();

            # query all contacts without a corresponding drupal user
            
            # nb: because we can't be sure if Civi lives in a different db to Drupal, or if
            # we have permission to do a cross-db join, doing two separate queries and an 
            # array_diff is prb the safest way to do this, although it could get a little slow 
            # with large numbers of contacts and users
            
            $uids       = array();
            $uf_ids     = array();
            $uf_matches = array();

            # get all uids from Drupal
            $results = db_select('user', 'u')
                ->fields('u', array('uid'))
                ->execute();
        
            foreach ($results as $result)
                $uids[] = $result->uid;

            # get all uf_ids from Civi
            $dao = CRM_Core_DAO::executeQuery("
                SELECT uf_id, contact_id FROM civicrm_uf_match
            ");
            while ($dao->fetch()) {
                $uf_ids[] = $dao->uf_id;
                # also keep track of the corresponding contact_id, 
                # so we don't need to look that up again
                $uf_matches[$dao->uf_id] = $dao->contact_id;
            }

            # get contact ids without a corresponding Drupal user
            $contact_ids = array();
            foreach (array_diff($uf_ids, $uids) as $uid)
                $contact_ids[] = $uf_matches[$uid]; 

            # create group
            if ($contact_ids) {

                $result = civicrm_api('group', 'create', array(
                    'version' => 3,
                    'name'    => $group_name = dt('Spambot CiviCRM Contacts - @date', array('@date' => date('dmYHiS')))
                ));

                if ($result['is_error']) {
                    
                    drush_log(dt(
                        'Unable to create group. @message', 
                        array('@message' => $result['error_message'])
                    ), 'error');

                } else { 

                    drush_log(dt('Created group: @name', array('@name' => $group_name)));

                    # put contacts in group
                    $params = array('version' => 3);
                    foreach ($contact_ids as $index => $contact_id)
                        $params['contact_id' . ($index ? $index + 1 : '')] = $contact_id;

                    $result = civicrm_api('groupcontact', 'create', $params);
                    
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