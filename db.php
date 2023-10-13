<?php

class Database {
 
    private $host = "localhost";
    private $db_name = "schooltrack";
    private $username = "ec2-user";
    private $password = "Eipl@1407";
    // database connection and table name
    private $conn;
 
    // constructor with database connection
    public function __construct() {
        $this->conn = null;
 
        try{
            $this->conn = new PDO("mysql:host=" . $this->host . ";dbname=" . $this->db_name, $this->username, $this->password);
            $this->conn->exec("set names utf8");
        }catch(PDOException $exception){
            echo "Connection error: " . $exception->getMessage();
        }
 
    }

    public function execute($query) {
    
        // prepare query statement
        $stmt = $this->conn->prepare($query);
    
        // execute query
        $stmt->execute();
    
        return $stmt;
    }

    public function register($device_id, $token) {
        // select all query
        $query = "UPDATE devices SET device_token='$token' 
                    WHERE id = $device_id
                ";
    
        // prepare query statement
        $stmt = $this->conn->prepare($query);
    
        // execute query
        $stmt->execute();

        //if( $stmt->rowCount() > 0 )
            return true;
        //else
        //    return false;
    }

    // read 
    public function read($device_id) {
    
        // select all query
        $query = "SELECT
                    id, device_name, latitude, longitude, battery_level, signal_level, last_update 
                FROM
                    devices WHERE id = $device_id
                ";
    
        // prepare query statement
        $stmt = $this->conn->prepare($query);
    
        // execute query
        $stmt->execute();
    
        return $stmt;
    }

    public function history($device_id, $limit) {
        $query = "SELECT
                latitude, longitude, date_time 
                FROM
                history where device_id = $device_id ORDER BY date_time DESC LIMIT $limit
                ";

        // prepare query statement
        $stmt = $this->conn->prepare($query);
    
        // execute query
        $stmt->execute();
    
        return $stmt;
    }

    public function dhistory($device_id, $limit) {
        // select all query
        $condition = "DATE(date_time) = DATE(NOW())";
        if($limit) {
            $condition = "DATE(date_time) = DATE(NOW() - INTERVAL $limit DAY)";
        }
        $query = "SELECT
                    latitude, longitude, date_time 
                FROM
                    history where device_id = $device_id AND $condition ORDER BY date_time DESC
                ";
        

        // prepare query statement
        $stmt = $this->conn->prepare($query);
    
        // execute query
        $stmt->execute();
    
        return $stmt;
    }

    public function fence_add($device_id, $fence_name, $latitude, $longitude, $radius) {
        // select all query
        $query = "INSERT INTO fencing
                    (device_id, fence_name, latitude, longitude, radius) 
                VALUES
                    ($device_id, '$fence_name', '$latitude', '$longitude', $radius)
                ";
    
        // prepare query statement
        $stmt = $this->conn->prepare($query);
    
        // execute query
        $stmt->execute();

        if( $stmt->rowCount() > 0 )
            return true;
        else
            return false;
    }

    public function fence_delete($device_id, $fence_id) {
        // select all query
        $query = "DELETE FROM fencing
                    WHERE device_id=$device_id AND id=$fence_id
                ";
    
        // prepare query statement
        $stmt = $this->conn->prepare($query);
    
        // execute query
        $stmt->execute();
        if( $stmt->rowCount() > 0 )
            return true;
        else
            return false;
    }

    public function fence_list($device_id) {
        // select all query
        $query = "SELECT
                    id, fence_name, latitude, longitude, radius 
                FROM
                    fencing where device_id = $device_id ORDER BY fence_name 
                ";
    
        // prepare query statement
        $stmt = $this->conn->prepare($query);
    
        // execute query
        $stmt->execute();
    
        return $stmt;
    }

    public function alerts($device_id, $alert_filter = 0) {
        // select all query
        $alert_subquery = "";
        if($alert_filter) 
            $alert_subquery = " AND alert_type=$alert_filter ";
        $query = "SELECT fence_name, date_time, exit_entry, alert_type FROM `alerts` left join fencing on alerts.fence_id=fencing.id 
             WHERE alerts.device_id = $device_id $alert_subquery
             ORDER by date_time DESC 
                ";
    
        // prepare query statement
        $stmt = $this->conn->prepare($query);
    
        // execute query
        $stmt->execute();
    
        return $stmt;
    }

    public function attendance($device_id, $fence_id, $month, $year) {
        $query = "SELECT date(date_time) as m_date, 
          sum(TIMESTAMPDIFF(MINUTE, date_time, 
          (SELECT min(date_time) from alerts a2 WHERE a2.date_time > a1.date_time AND DATE(a2.date_time)=DATE(a1.date_time) 
          AND exit_entry=-1 AND device_id=$device_id AND fence_id=$fence_id))) as mins 
          FROM `alerts` a1 where exit_entry = 1 AND device_id=$device_id AND fence_id=$fence_id 
          AND MONTH(date_time)=$month AND YEAR(date_time)=$year group by m_date
                ";
    
        // prepare query statement
        $stmt = $this->conn->prepare($query);
    
        // execute query
        $stmt->execute();
    
        return $stmt;
    }

}