<?php

/**
 * @file
 * The PHP page that serves all page requests on a Drupal installation.
 *
 * The routines here dispatch control to the appropriate handler, which then
 * prints the appropriate page.
 *
 * All Drupal code is released under the GNU General Public License.
 * See COPYRIGHT.txt and LICENSE.txt.
 */

if( $_SERVER['REQUEST_URI'] == "/optin?message=doneoptin"   ||  $_SERVER['REQUEST_URI'] == "/error"  ||
	$_SERVER['REQUEST_URI'] == "/thanks"  ||  $_SERVER['REQUEST_URI'] == "/welcome"
	||
	$_SERVER['REQUEST_URI'] == "/optin?message=doneoptin/"   ||  $_SERVER['REQUEST_URI'] == "/error/"  ||
	$_SERVER['REQUEST_URI'] == "/thanks/"  ||  $_SERVER['REQUEST_URI'] == "/welcome/" )
{
	$paramPos = strpos( $_SERVER['REQUEST_URI'], '?' );
	if( !$paramPos )
		header( "Location: http://bulle-baer.com/landing".$_SERVER['REQUEST_URI'] );
		
	else
	{
		if( substr( $_SERVER['REQUEST_URI'], -1, 1 ) == "/" )
			header( "Location: http://bulle-baer.com/landing".substr( $_SERVER['REQUEST_URI'], 0, $paramPos-1 ) );
		else
			header( "Location: http://bulle-baer.com/landing".substr( $_SERVER['REQUEST_URI'], 0, $paramPos ) );
	}
}

else
{
	
	require_once './includes/bootstrap.inc';
	drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
	
	$return = menu_execute_active_handler();
	
	// Menu status constants are integers; page content is a string.
	if (is_int($return)) {
	  switch ($return) {
	    case MENU_NOT_FOUND:
	      drupal_not_found();
	      break;
	    case MENU_ACCESS_DENIED:
	      drupal_access_denied();
	      break;
	    case MENU_SITE_OFFLINE:
	      drupal_site_offline();
	      break;
	  }
	}
	elseif (isset($return)) {
		// Print any value (including an empty string) except NULL or undefined:
		print theme('page', $return);
	}
	
	drupal_page_footer();
}
