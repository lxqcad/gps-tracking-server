<?php
    function calculateDistance($latitude1, $longitude1, $latitude2, $longitude2) {
        $latitude1 = deg2rad($latitude1);
        $longitude1 = deg2rad($longitude1);
        $latitude2 = deg2rad($latitude2);
        $longitude2 = deg2rad($longitude2);
        $c = sin(($latitude1)) * sin(($latitude2)) + cos(($latitude1)) * cos(($latitude2)) * cos(($longitude2) - ($longitude1));
        $c = $c > 0 ? min(1, $c) : max(-1, $c);
        return 3959 * 1.609 * 1000 * acos($c);
    }
    
    $lat = 26.86790300;
    $lng = 81.02115000;
    $lat1 = 26.88557333;
    $lng1 = 80.99518222;
    $radius = 22;
    echo "\nDistance = ".calculateDistance($lat,$lng,$lat1,$lng1)."\n";
    //$device_id = $row['device_id'];
    //$fence_id = $row['id'];

    $r1 = 5;

    $dLat = ($lat1-$lat) * pi() / 180;
    $dLon = ($lng1-$lng) * pi() / 180;
    
    $a1 = sin($dLat/2) * sin($dLat/2) + cos($lat * pi() / 180 ) * cos($lat1 * pi() / 180 ) * sin($dLon/2) * sin($dLon/2);
    $c1 = 2 * atan2(sqrt($a1), sqrt(1 - $a1));
    $d1 = $r1 * $c1;
    if($d1 > $radius) {
        echo " Outside.($d1)\n";
    }
    else {
        echo " Inside.($d1)\n";
    }
?>