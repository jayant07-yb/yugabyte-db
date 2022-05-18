---
title: Camunda
linkTitle: Camunda
description: Use Camunda with YSQL 
aliases:
section: INTEGRATIONS
menu:
  preview:
    identifier: camunda
    weight: 571
isTocNested: true
showAsideToc: true
---

[Camunda](https://camunda.com/) is a Java-based framework supporting BPMN (Business Process Modeling Notation) for workflow and process automation, CMMN(Case Management Model and Notation) for Case Management and DMN(Decision Model and Notation) for Business Decision Management. 
While using Camunda we are required to configure a datasource. In this doc we discuss how YugabyteDB (ysql) can be used as a datasource in Camunda.

## Prerequisites
Make sure you have the following set of tools installed:

- Java JDK 1.8+
- NodeJS >= v10 
- Camunda Modeler, you can get it from [here](https://camunda.com/download/modeler).

## Configure Camunda Platform 7
- Start the YugabyteDB cluster.
  Refer to [YugabyteDB Quick start guide](/preview/quick-start/) to install and start a local cluster.
  Or you can use Docker container 
  `docker run -p7000:7000 -p9000:9000 -p5433:5433 -p9042:9042 yugabytedb/yugabyte:latest bin/yugabyted start\`
  Refer [here](https://hub.docker.com/r/yugabytedb/yugabyte) for more details regarding the docker image.
- Download Camunda Platform 7 from [here](https://camunda.com/download/) and unzip it.

- In the following configuration files find the spring.datasource section and replace it with the appropiate configurations
  - `camunda-bpm-run-7.17.0/configuration/default.yml`
  - `camunda-bpm-run-7.17.0/configuration/production.yml`

```yml
# datasource configuration is required
spring.datasource:
 url: jdbc:postgresql://localhost:5433/yugabyte
 driver-class-name: org.postgresql.Driver
 username: yugabyte
 password: yugabyte
```
Change the `connection url` to point to the YugabyteDB cluster you started.
Download the Postgres JDBC driver jar and place it in `camunda-bpm-run-7.17.0/configuration/userlib` directory.
You can get the jar [file](https://jdbc.postgresql.org/download/postgresql-42.3.5.jar).
This configuration uses Postgres JDBC driver to connect with the yugabyteDB, for using YugabyteDB JDBC driver 
Use the below configuration.
```yml
# datasource configuration is required
spring.datasource:
 url: jdbc:yugabytedb://localhost:5433/yugabyte
 driver-class-name: com.yugabyte.Driver
 username: yugabyte
 password: yugabyte
```
Download the YugabyteDB JDBC driver jar and place it in `camunda-bpm-run-7.17.0/configuration/userlib` directory.
You can get the jar [file](https://repo1.maven.org/maven2/com/yugabyte/jdbc-yugabytedb/42.3.5-yb-1/jdbc-yugabytedb-42.3.5-yb-1.jar).

You can read more about YugabyteDB JDBC driver [here](https://docs.yugabyte.com/preview/integrations/jdbc-driver/).

Run the Camunda Platform server using `./start.sh` for Linux/MacOS systems or `./start.bat` for Windows System.

## Verify the integration
Once server is started you will be able to see a few tables getting created in the database. To do so, login to the database using ysqlsh
`./bin/ysqsh`
Use `\d` to see the list of tables.
A list of tables with the prefix `ACT_` will appear.
More details regarding the tables created are mentioned [here](https://docs.camunda.org/manual/7.16/user-guide/process-engine/database/database-schema/).

## Hello World Application using Camunda
We are going to use Camunda Modeler to design and deploy a simple BPMN for charging the cards. This example is taken from the [quick-start example](https://docs.camunda.org/get-started/quick-start/) in Camunda Docs.
We are going to need 3 events:
- Start Event   
- Service Event
- End Event

For Service Event we are going to need an external task worker, for that purpose we are going to use `Nodejs`.
Download the Camunda Modeler from [here](https://camunda.com/download/modeler) and launch it.

- Create a new BPMN diagram by clicking File > New File > BPMN Diagram.
![alt text](https://docs.camunda.org/get-started/quick-start/img/modeler-new-bpmn-diagram.png)
- Double-click on the Start Event. A text box will open. Name the Start Event “Payment Retrieval Requested”.
- Following it create a service event.
  Click on the start event. From its context menu, select the activity shape (rounded rectangle). It will be placed automatically on the canvas, and you can drag it to your preferred position. Name it Charge Credit Card. Change the activity type to Service Task by clicking on the activity shape and using the wrench button.
![alt text](https://docs.camunda.org/get-started/quick-start/img/modeler-step2.png)
- Similarly add an End Event named Payment Received.
![alt text](https://docs.camunda.org/get-started/quick-start/img/modeler-step3.png)
- We are now required to configure the service task.
![alt text](https://docs.camunda.org/get-started/quick-start/img/modeler-step5.png)
  - First, configure an ID for the process. Type payment-retrieval in the property field Id. The property ID is used by the process engine as an identifier for the executable process, and it’s best practice to set it to a human-readable name.
  - Second, configure the Name of the process. Type Payment Retrieval in the property field Name.
  - Finally, make sure the box next to the Executable property is checked. If you don’t check this box, the process definition is ignored by the process engine.
- Save the BPMN Diagram
- Implement an external task worker
After modeling the process, we want to execute some business logic.
Camunda Platform is built so that your business logic can be implemented in different languages. You have the choice which language suits your project best. We are going to use JavaScript(NodeJs)
- Create a new NodeJS project
```bash
mkdir charge-card-worker
cd ./charge-card-worker
npm init -y
```
- Add Camunda External Task Client JS library
```bash
npm install camunda-external-task-client-js
npm install -D open
```

Implement the NodeJS script
Next, we’ll create a new ExternalTaskClient that subscribes to the charge-card topic.

When the process engine encounters a service task that’s configured to be externally handled, it creates an external task instance on which our handler will react. We use Long Polling in the ExternalTaskClient to make the communication more efficient.

Next, you need to create a new JavaScript file, e.g. worker.js, that looks like the following:
```js
const { Client, logger } = require('camunda-external-task-client-js');
const open = require('open');

// configuration for the Client:
//  - 'baseUrl': url to the Process Engine
//  - 'logger': utility to automatically log important events
//  - 'asyncResponseTimeout': long polling timeout (then a new request will be issued)
const config = { baseUrl: 'http://localhost:8080/engine-rest', use: logger, asyncResponseTimeout: 10000 };

// create a Client instance with custom configuration
const client = new Client(config);

// susbscribe to the topic: 'charge-card'
client.subscribe('charge-card', async function({ task, taskService }) {
  // Put your business logic here

  // Get a process variable
  const amount = task.variables.get('amount');
  const item = task.variables.get('item');

  console.log(`Charging credit card with an amount of ${amount}€ for the item '${item}'...`);

  open('https://docs.camunda.org/get-started/quick-start/success');

  // Complete the task
  await taskService.complete(task);
});
```
- Run the NodeJS script
  `node ./worker.js`

- Deploy the BPMN using Camunda Modeler
  In order to deploy the Process, click on the deploy button in the Camunda Modeler, then give it the Deployment Name “Payment Retrieval” and click the Deploy button. From version 3.0.0 on, you will be required to provide an URL for an Endpoint Configuration along with Deployment Details. This can be either the root endpoint to the REST API (e.g. http://localhost:8080/engine-rest) or an exact endpoint to the deployment creation method (e.g. http://localhost:8080/engine-rest/deployment/create).
![alt text](https://docs.camunda.org/get-started/quick-start/img/modeler-deploy2.png)
- Verify the Deployment with Cockpit
 Go to http://localhost:8080/camunda/app/cockpit/ and log in with the credentials demo / demo. Your process Payment Retrieval should be visible on the dashboard.
- Start a Process Instance 
  We can can leverage the Camunda REST API to start a new process instance by sending a POST Request. 
  We can use the following curl command 
  `curl -H "Content-Type: application/json" -X POST -d '{"variables": {"amount": {"value":555,"type":"integer"}, "item": {"value":"item-xyz"} } }' http://localhost:8080/engine-rest/process-definition/key/payment-retrieval/start`
  or we can make a POST request from postman
  The JSON Body should look like this:
```json
{
	"variables": {
		"amount": {
			"value":555,
			"type":"integer"
		},
		"item": {
			"value": "item-xyz"
		}
	}
} 
```
- In your worker, you should now see the output in your console. This means you have successfully started and executed your first simple process.

 