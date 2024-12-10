let mqttClient;

window.addEventListener("load", (event) => {
  connectToBroker();

  const onBtn = document.querySelector(".on");
  onBtn.addEventListener("click", function () {
    publishOnMessage();
  });
  const offBtn = document.querySelector(".off");
  offBtn.addEventListener("click", function () {
    publishOffMessage();
  });
});

function connectToBroker() {
  const clientId = "client" + Math.random().toString(36).substring(7);

  // Change this to point to your MQTT broker
  const host = "ws://j4n-u5.local:8080";

  const options = {
    keepalive: 60,
    clientId: clientId,
    protocolId: "MQTT",
    protocolVersion: 4,
    clean: true,
    reconnectPeriod: 1000,
    connectTimeout: 30 * 1000,
  };

  mqttClient = mqtt.connect(host, options);

  mqttClient.on("error", (err) => {
    console.log("Error: ", err);
    mqttClient.end();
  });

  mqttClient.on("reconnect", () => {
    console.log("Reconnecting...");
  });

  mqttClient.on("connect", () => {
    console.log("Client connected:" + clientId);
  });

  // Received
  mqttClient.on("message", (topic, message, packet) => {
    console.log(
      "Received Message: " + message.toString() + "\nOn topic: " + topic
    );
  });
}

function publishOnMessage() {

  const topic = "light/command";
  const message = "on";

  console.log(`Sending Topic: ${topic}, Message: ${message}`);

  mqttClient.publish(topic, message, {
    qos: 0,
    retain: false,
  });
}

function publishOffMessage() {

  const topic = "light/command";
  const message = "off";
  console.log(`Sending Topic: ${topic}, Message: ${message}`);

  mqttClient.publish(topic, message, {
    qos: 0,
    retain: false,
  });
}
