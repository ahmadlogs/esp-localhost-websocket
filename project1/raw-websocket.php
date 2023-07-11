<?php  
require 'vendor/autoload.php';
use Ratchet\MessageComponentInterface;  
use Ratchet\ConnectionInterface;

class Chat implements MessageComponentInterface {  
    public $clients;
    private $logs;
    private $connectedUsers;
    private $connectedUsersNames;

    public function __construct() {
        $this->clients = new \SplObjectStorage;
        $this->logs = [];
        $this->connectedUsers = [];
        $this->connectedUsersNames = [];
    }

    public function onOpen(ConnectionInterface $conn) {
        $this->clients->attach($conn);
        echo "New connection! ({$conn->resourceId})\n";
        //$conn->send(json_encode($this->logs));
        $this->connectedUsers [$conn->resourceId] = $conn;
    }

    public function onMessage(ConnectionInterface $from, $msg) {
        // Do we have a username for this user yet?
        if (isset($this->connectedUsers[$from->resourceId])) {
			
			// Split the string into an array using ":" as the delimiter
			$data = explode(":", $msg);
			
			// Create an associative array with the key-value pairs
			$jsonData = array(
				"msg"  		=> isset($data[0]) ? $data[0] : "",
				"sensor"	=> isset($data[1]) ? $data[1] : "",
				"receiver" 	=> isset($data[2]) ? $data[2] : "",
				"sender" 	=> isset($data[3]) ? $data[3] : ""
			);
			
            $this->sendMessage($jsonData);
        } //else {
            // If we don't this message will be their username
            //$this->connectedUsers[$from->resourceId] = $from;
        //}
		
		// Send data back to the client
		//$from->send("Response message");
        
		//broadcast the message to every client 
		//foreach ($this->clients as $client) {
            //$client->send($msg);
        //}
		
		// Broadcast the message to every client except the one that sent it (ESP32)
		//foreach ($this->clients as $client) {
			//if ($client !== $from) {
				//$client->send($msg);
			//}
		//}
    }

    public function onClose(ConnectionInterface $conn) {
        // Detatch everything from everywhere
        $this->clients->detach($conn);
        unset($this->connectedUsersNames[$conn->resourceId]);
        unset($this->connectedUsers[$conn->resourceId]);
    }

    public function onError(ConnectionInterface $conn, \Exception $e) {
        $conn->close();
    }

    private function sendMessage($message) {
        foreach ($this->connectedUsers as $user) {
            $user->send(json_encode($message));
        }
    }
}

// Run the server application through the WebSocket protocol on port 8080
//$app = new Ratchet\App("localhost", 8080, '0.0.0.0', $loop);
//REPLACE WITH YOUR COMPUTER IP ADDRESS WHERE THE WEBSOCKET SERVER IS RUNNING
$app = new Ratchet\App("IP_ADDRESS", 81, "0.0.0.0");
$app->route('/', new Chat, array('*'));

$app->run();
