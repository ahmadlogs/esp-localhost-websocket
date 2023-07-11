<?php
require 'vendor/autoload.php';

use Ratchet\MessageComponentInterface;
use Ratchet\ConnectionInterface;

class MyWebSocket implements MessageComponentInterface { 
	public $clients;
	private $connectedClients;
	
    public function __construct() {
        $this->clients = new \SplObjectStorage;
        $this->connectedClients = [];
    }
	
	public function onOpen(ConnectionInterface $conn) { 
		$this->clients->attach($conn);
		$this->connectedClients[$conn->resourceId] = $conn;
		echo "New connection! ({$conn->resourceId})\n";
		$conn->send("Welcome to the Server.");
	}
	
	public function onMessage(ConnectionInterface $from, $msg) { 
		//echo $msg . "\n";
		foreach ($this->connectedClients as $client) { 
			$client->send($msg);
		}
	}
	
	public function onClose(ConnectionInterface $conn) { 
		// Detatch everything from everywhere
        $this->clients->detach($conn);
        unset($this->connectedClients[$conn->resourceId]);
	}
	
	public function onError(ConnectionInterface $conn, \Exception $e) { 
		echo "An error occurred: " . $e->getMessage() . "\n";
		$conn->close();
	}
}

//REPLACE WITH YOUR COMPUTER IP ADDRESS WHERE THE WEBSOCKET SERVER IS RUNNING
$app = new Ratchet\App("IP_ADDRESS", 81, "0.0.0.0");
$app->route('/', new MyWebSocket, array('*'));

$app->run();
?>
























