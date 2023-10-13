<?php
/*
 * Copyright 2018-2019 by Juby Abraham
 * GPS Tracking 
 * Cron for GPS Track to update device status and clean up old history running @every 2 min
 * Emulate Infotech Pvt. Ltd.
 *
 */

require_once "db.php";
require_once "firebase.php";
require_once "push.php"; 

define("OFFLINE", 3);
define("A_OFFLINE", 0x04);


$database = new Database();

$database->execute("DELETE FROM `history` WHERE `date_time` < (NOW() - INTERVAL 5 DAY)");
$database->execute("DELETE FROM `alerts` WHERE `date_time` < (NOW() -  INTERVAL 6 MONTH)");

$stmt = $database->execute("SELECT id, device_name, device_token, notifications FROM `devices` WHERE time_to_sec(timediff(now(),last_update)) > 240;");

while ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
    $device_id = $row['id'];
    $notification = $row['notifications'];
    if($notification & A_OFFLINE) {
        $stmt_2 = $database->execute("SELECT COUNT(*) as num_alerts FROM `alerts` WHERE device_id=$device_id AND alert_type=".OFFLINE." AND date_time > NOW() - INTERVAL 15 MINUTE");
        if($row_2 = $stmt_2->fetch(PDO::FETCH_ASSOC)) {
            if(intval($row_2['num_alerts']) == 0) {
                $device_name = $row['device_name'];
                $device_token = array($row['device_token']);
                if($device_token !== '') {
                    echo $device_name." is currently OFFLINE.\n";
                    $database->execute("INSERT INTO alerts (device_id, exit_entry, fence_id, alert_type) VALUES ($device_id, 0, 0, ".OFFLINE.")");

                    $push = new Push(
                        'DEVICE OFFLINE',
                        $device_name . ' is currently OFFLINE due to Signal or Battery issue.',
                        'https://emulate.ml/tracking/images/device-offline.png'
                    );

                    //getting the push from push object
                    $mPushNotification = $push->getPush(); 
                
                    //creating firebase class object 
                    $firebase = new Firebase(); 
                
                    //sending push notification and displaying result 
                    echo $firebase->send($device_token, $mPushNotification);

                }
            }
        }
    }
} 


?>
