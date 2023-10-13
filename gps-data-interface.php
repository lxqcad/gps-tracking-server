<?php
/*
 * Copyright 2018-2019 by Juby Abraham
 * GPS Tracking 
 * Database Interface for GPS Track Server Code 
 * Emulate Infotech Pvt. Ltd.
 *
 */

require_once "db.php";
require_once "firebase.php";
require_once "push.php"; 

define("SOS", 0);
define("FenceIN", 1);
define("FenceOUT", -1);
define("OFFLINE", 3);
define("IDLE", 4);
define("OVERSPEED", 5);

define("A_SOS", 0x01);
define("A_FENCE", 0x02);
define("A_OFFLINE", 0x04);
define("A_IDLE", 0x08);
define("A_OVERSPEED", 0x10);
define("A_ALL", 0x1F);

$database = new Database();

$imei = "";
$lat = 0.0;
$lng = 0.0;
$velocity = 0;

$voltage = 0;
$gsmlevel = 0;

$msg_type = $argv[1];
$imei = $argv[2];

$device_id = 0;

function send_firebase_message($alert_type, $fence_name='')
{
    global $imei, $database, $velocity;
    $push = null; 
    $notification = 0; 
    $note_apply = 0;

    $stmt2 = $database->execute("SELECT device_name, device_token, notifications FROM devices WHERE imei_number=$imei");

    if($stmt2->rowCount()) {
        $row1 = $stmt2->fetch(PDO::FETCH_ASSOC);
        //extract($row);

        $device_name = $row1['device_name'];
        $device_token = array($row1['device_token']);
        $notification = $row1['notifications'];
    
        switch($alert_type){
            case SOS:      // SOS Alert
                $push = new Push(
                    'SOS Alert',
                    'SOS help request from device registered as ' . $device_name . '.',
                    'https://emulate.ml/tracking/images/sos.jpg'
                );
                $note_apply = A_SOS;
                break;
            case IDLE:  // DEVICE IDLE
                $push = new Push(
                    'DEVICE IDLE',
                    $device_name . ' is currently IDLE at last location.',
                    'https://emulate.ml/tracking/images/device-idle.png'
                );
                $note_apply = A_IDLE;
                break;
            case FenceIN:  // Fence IN
                $push = new Push(
                    $fence_name.' ENTER',
                    $device_name . ' just entered '.$fence_name.' area',
                    'https://emulate.ml/tracking/images/fence-in.png'
                );
                $note_apply = A_FENCE;
                break;
            case FenceOUT: // Fence OUT
                $push = new Push(
                    $fence_name.' EXIT',
                    $device_name . ' has left '.$fence_name.' area',
                    'https://emulate.ml/tracking/images/fence-out.png'
                );
                $note_apply = A_FENCE;
                break;
            case OVERSPEED: // Overspeed
                $push = new Push(
                    'OVERSPEED',
                    $device_name . ' is overspeeding at '.$velocity.' km/hr',
                    'https://emulate.ml/tracking/images/overspeed-alert.png'
                );
                $note_apply = A_OVERSPEED;
                break;
            
        }

        if($notification & $note_apply) {
            //getting the push from push object
            $mPushNotification = $push->getPush(); 
            
            //creating firebase class object 
            $firebase = new Firebase(); 
            
            //sending push notification and displaying result 
            echo $firebase->send($device_token, $mPushNotification);
        }
    }
    
}

function calculate_distance($latitude1, $longitude1, $latitude2, $longitude2) 
{
    $latitude1 = deg2rad($latitude1);
    $longitude1 = deg2rad($longitude1);
    $latitude2 = deg2rad($latitude2);
    $longitude2 = deg2rad($longitude2);
    $c = sin(($latitude1)) * sin(($latitude2)) + cos(($latitude1)) * cos(($latitude2)) * cos(($longitude2) - ($longitude1));
    $c = $c > 0 ? min(1, $c) : max(-1, $c);
    return 3959 * 1.609 * 1000 * acos($c);
}


function update_gps()
{
    global $imei, $device_id, $lat, $lng, $database;

    if($lat > 1) {
        $database->execute("INSERT INTO history (device_id, latitude, longitude) SELECT id, $lat, $lng FROM devices WHERE imei_number=$imei AND (latitude <> '$lat' OR longitude <> '$lng')");
        $database->execute("UPDATE devices SET latitude='$lat', longitude='$lng', last_update=NOW() WHERE imei_number=$imei");

        $stmt = $database->execute("SELECT id, fence_name, latitude, longitude, radius FROM fencing WHERE device_id=$device_id");

        while ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
            // extract row
            $lat1 = $row['latitude'];
            $lng1 = $row['longitude'];
            $radius = $row['radius'];
            $fence_id = $row['id'];
            $fence_name = $row['fence_name'];

            $d1 = calculate_distance($lat, $lng, $lat1, $lng1);
            $stmt_ee = $database->execute("SELECT Sum(exit_entry) As exit_entry_sum FROM alerts WHERE device_id=$device_id AND fence_id=$fence_id");
            if ($row_ee = $stmt_ee->fetch(PDO::FETCH_ASSOC)) {
                $exit_entry_sum = $row_ee['exit_entry_sum'];
            }
            if($d1 > $radius) {
                echo $row['fence_name']." Outside.($d1)".pi()."\n";
                if($exit_entry_sum > 0) {
                    $database->execute("INSERT INTO alerts (device_id, exit_entry, fence_id, alert_type) VALUES ($device_id, -1, $fence_id, 2)");
                    send_firebase_message(FenceOUT, $fence_name);
                }
            }
            else {
                echo $row['fence_name']." Inside.($d1)".pi()."\n";
                if($exit_entry_sum == 0) {
                    $database->execute("INSERT INTO alerts (device_id, exit_entry, fence_id, alert_type) VALUES ($device_id, 1, $fence_id, 2)");
                    send_firebase_message(FenceIN, $fence_name);
                }
            }
        }
    }
    else {
        $database->execute("UPDATE devices SET last_update=NOW() WHERE imei_number=$imei");
    }
    //$stmt = $database->execute("SELECT time_to_sec(timediff(now(),max(date_time)))/60 as diff1 FROM `history` WHERE device_id=$device_id");
    //if($stmt->rowCount()) {

}

$stmt = $database->execute("SELECT id FROM devices WHERE imei_number=$imei");
if ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
    $device_id = $row['id'];
} 
else {
    printf("Device with imei [%s] is not found", $imei);
    if (strpos($imei, '.') === false) {
        $database->execute("INSERT IGNORE INTO unregistered (imei) VALUES ($imei)");
    }
    else
        echo 'INCORRECT IMEI RETURNED\n';
    exit(-1);
}

if($device_id) {
    switch($msg_type) {
        case "gps":
            $lat = $argv[3];
            $lng = $argv[4];
            $velocity = $argv[5];
            update_gps();
            // file_put_contents("log.txt", "\n\r".date("y-m-d H:i:s")." => Speed = ".$velocity, FILE_APPEND);
            if($velocity > 60) {
                $database->execute("INSERT INTO alerts (device_id, exit_entry, fence_id, alert_type, latitude, longitude) VALUES ($device_id, 0, 0, 5, '$lat', '$lng')");
                send_firebase_message(OVERSPEED);
            }
            break;    
        case "heartbeat":
            $voltage = $argv[3];
            $voltage = intval($voltage * 100 / 6);
            $gsmlevel = $argv[4];
            $database->execute("UPDATE devices SET last_update=NOW(), battery_level=$voltage, signal_level=$gsmlevel WHERE imei_number=$imei");
            break;
        case "alarm":
            $lat = $argv[3];
            $lng = $argv[4];
            $alarm_type = $argv[5];
            update_gps();
            printf("\nAlarm Type = %d\n", $alarm_type);
//            if($alarm_type == 1) {
            $database->execute("INSERT INTO alerts (device_id, exit_entry, fence_id, alert_type, latitude, longitude) VALUES ($device_id, 0, 0, 1, '$lat', '$lng')");
            send_firebase_message(SOS);
/*            }
            if($alarm_type == 6) {
                $database->execute("INSERT INTO alerts (device_id, exit_entry, fence_id, alert_type, latitude, longitude) VALUES ($device_id, 0, 0, 5, '$lat', '$lng')");
                send_firebase_message(OVERSPEED);
            }
*/
            break;
    }

    $stmt = $database->execute("SELECT COUNT(*) as num_alerts FROM `alerts` WHERE device_id=$device_id AND (alert_type=".IDLE." OR alert_type=".OFFLINE.") AND date_time > NOW() - INTERVAL 15 MINUTE");
        
    if($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
        if(intval($row['num_alerts']) == 0) {
            $stmt = $database->execute("SELECT time_to_sec(timediff(now(),max(date_time))) as diff1 FROM `history` WHERE device_id=$device_id");
            if($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
                if(intval($row['diff1']) > 300) {
                    $stmt = $database->execute("SELECT Sum(exit_entry) As exit_entry_sum FROM alerts WHERE device_id=$device_id AND alert_type=2");
                    if ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
                        if(intval($row['exit_entry_sum']) == 0) {
                            $database->execute("INSERT INTO alerts (device_id, exit_entry, fence_id, alert_type) VALUES ($device_id, 0, 0, ".IDLE.")");
                            send_firebase_message(IDLE);
            
                        }
                    }
                }
            }
        }
    }
}

//printf("%s = %.8f, %.8f", $imei, $lat, $lng);

?>
