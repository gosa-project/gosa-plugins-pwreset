<?php

/*
  This code is an addon for GOsa (https://gosa.gonicus.de)
  Copyright (C) 2015 Mike Gabriel

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

class managepws extends plugin
{
    /* Definitions */
    var $plHeadline= "Password Management Add-On";
    var $plDescription= "GOsa2 Password Management Add-On";
    var $access= "";

    /* Array with csv informations*/
    var $csvinfo=array();

    /* attribute list for save action */
    var $attributes= array();
    var $objectclasses= array();
    var $view_logged = FALSE;

    function managepws (&$config, $dn= NULL)
    {
        $this->initTime = microtime(TRUE);

        /* Include config object */
        $this->config= &$config;

        $this->ui = get_userinfo();
        stats::log('plugin', $class = get_class($this), $category = array($this->acl_category),  $action = 'open',
                   $amount = 1, $duration = (microtime(TRUE) - $this->initTime));
    }

    function getAttributes()
    {
        /* columns in pw reset CSV */
        $attrs= array ();

	/* passwords can be reset for uid DN objects or cn DN objects */
        $attrs[0]= "uid";
        $attrs[1]= "cn";
        $attrs[2]= "userPassword";

        return $attrs;
    }

    function execute()
    {

        /* Call parent execute */
        plugin::execute();

        /* Log view */
        if(!$this->view_logged){
            $this->view_logged = TRUE;
            new log("view","all/".get_class($this),$this->dn);
        }

        /* initiate smarty */
        $smarty= get_smarty();

        /* PHASE 01 done, set $pwreset_configured to TRUE  */
        $smarty->assign ("pwreset_configured",0);

        /* PHASE 02 done, set $pwreset_reviewed to TRUE  */
        $smarty->assign ("pwreset_reviewed",0);

        /* Get the LDAP link, to generate the Export */
        $this->_ldap=$this->config->get_ldap_link ();

        /* initialize CSV Info array */
        if (!is_array ($this->csvinfo)) {
            $this->csvinfo=array ();
        }

        /* Check permissions for pwreset */
        $acl=$this->ui->get_permissions ($this->config->current['BASE'],"all/all");
        if (!preg_match ("/w/",$acl)) {
            if (isset ($_POST['userfile'])||isset ($_POST['sorted'])||isset ($_POST['fileup'])) {
                msg_dialog::display (_ ("Permission error"),_ ("You've no permission to import CSV files!"),ERROR_DIALOG);
            }
            return ($smarty->fetch (get_template_path ('content_manageaccounts.tpl',TRUE)));
        }

        /*
         * PHASES
         */

        /* reset our failure status from last template view... */
        $this->failure_in_this_phase = FALSE;

        /*
         * prepare PHASE 1
         */

        if ((!isset ($_POST['phase_01'])) && (!isset ($_POST['phase_02']))) {

            /* Search OUs with objectClass gosaDepartment */
            $this->ou_tree = array();
            $this->ou_tree['ldapsearch'] = $this->_ldap->search("(objectClass=gosaDepartment)",array("ou","description"));

            /* create arrays for search results */
            $this->ou_tree['formfields'] = array();
            $this->ou_tree['OUs'] = array();

            /* add found gosaDepartment objects */
            while($result = $this->_ldap->fetch($this->ou_tree['ldapsearch'])) {
                $this->ou_tree['OUs'][] = $result['ou'][0];
                $this->ou_tree['formfields'][] = $result['ou'][0]." - ".$result['description'][0];
                $this->ou_tree['DNs'][] = $result['dn'];
            }

            $smarty->assign("ous_available",$this->ou_tree['formfields']);

            /* pwreset configuration defaults */
            $this->pwreset_mode= 0;

            /* provide pre-set values for account template forms */
            $smarty->assign("preset_pwreset_mode", $this->pwreset_mode);
        }

        /*
         * PHASE 02
         */
        elseif (isset ($_POST['phase_01'])) {

            $this->csvinfo['attrs']=$this->getAttributes();
            $this->csvinfo['attrs'][] = "---";

            /* configure options */
            if (isset ($_POST["pwreset_mode"]))
            {
            }

            if($this->failure_in_this_phase===FALSE)
            {
                $smarty->assign ("pwreset_configured",TRUE);
            }
            else {
                /* prepare for reloading this phase's web page again */
                $smarty->assign("preset_pwreset_mode", $this->pwreset_mode);
            }
        }

        /*
         * PHASE 02
         */
        elseif (isset ($_POST['phase_02'])) {
            $smarty->assign ("pwreset_configured",TRUE);

            /* Check if theres a file uploaded */
            if (!empty ($_FILES['userfile']['name'])) {
                $handle=NULL;
                $filename=gosa_file_name ($_FILES['userfile']['tmp_name']);
                if ( (!isset ($_FILES['userfile']['name']))|| (!isset ($_POST['fileup']))) {
                    msg_dialog::display (_ ("Error"),sprintf (_ ("Cannot read uploaded file: %s"),_ ("file not found")),ERROR_DIALOG);
                    $smarty->assign ("LDIFError",TRUE);
                }
                elseif (!$_FILES['userfile']['size']>0) {
                    msg_dialog::display (_ ("Error"),sprintf (_ ("Cannot read uploaded file: %s"),_ ("file is empty")),ERROR_DIALOG);
                    $smarty->assign ("LDIFError",TRUE);
                }
                /* Is there a tmp file, which we can use ? */
                elseif (!file_exists ($filename)) {
                    msg_dialog::display (_ ("Error"),sprintf (_ ("Cannot read uploaded file: %s"),_ ("file not found")),ERROR_DIALOG);
                    $smarty->assign ("LDIFError",TRUE);
                }
                elseif (!$handle=@fopen ($filename,"r")) {
                    msg_dialog::display (_ ("Error"),sprintf (_ ("Cannot read uploaded file: %s"),_ ("file not readable")),ERROR_DIALOG);
                    $smarty->assign ("LDIFError",TRUE);
                }
                else {
                    $smarty->assign ("file_uploaded",TRUE);
                    $raw_csv_data="";

                    /* Reading content */
                    while (!feof ($handle)) {
                        $raw_csv_data.=fread ($handle,1024);
                    }

                    @fclose ($handle);

                    if(!mb_check_encoding($raw_csv_data, "UTF-8"))
                    {
                        if(mb_check_encoding($raw_csv_data, "iso-8859-1"))
                        {
                            $raw_csv_data = utf8_encode($raw_csv_data);
                        } else
                        {
                            $smarty->assign ("LDIFError",TRUE);
                            $smarty->assign ("file_uploaded",FALSE);
                            msg_dialog::display (_ ("Error"),_ ("File has an unsupported encoding!"),ERROR_DIALOG);
                        }
                    }

                    $parser_ok = pwreset::parseCSV($raw_csv_data);

                    if ($parser_ok!==FALSE)
                    {
                    }
                }
            }
        }

        /* Show main page */
        return ($smarty->fetch (get_template_path('content_managepws.tpl', TRUE)));
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
