<?php

$z = new zookeeper("localhost:2181"); 

$services = $z->getChildren("/service-location");
echo "Found services: " . implode(', ', $services) . "\n";

foreach ($services as $service) {
        echo " Getting nodes providing '$service'\n";
        $nodes = $z->getChildren("/service-location/$service");
        echo " Found nodes: " . implode(', ', $nodes) . "\n";

        foreach ($nodes as $node) {
            echo "  $service on $node has address " . $z->get("/service-location/$service/$node") . "\n";
        }

}
