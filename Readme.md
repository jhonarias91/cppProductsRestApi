### C++ Simple Rest API

### Sources

- Make sure you have a IDE like VSCode

- *Instalar todas las depdendencias necesarias. como g++ (compilador c++) librerias web libboost-all-dev cpprest es una libreria para REST Api* 
 estas librerias y compilador se deben instalar en el ambiente donde se desee compilar y correr la api.
    
```sudo apt update```

```sudo apt install -y git g++ libboost-all-dev libssl-dev libcpprest-dev libgtest-dev```

*Verificar instalación*

```dpkg -l | grep libcpprest```

- Clonar el proyecto:

 ```git clone https://github.com/jhonarias91/cppProductsRestApi.git```

- Dar permisos a la carpeta 

```sudo chmod -R 777 cppProductsRestApi```

- *Compilar el programa*

En la carpeta src:
    
```g++ -std=c++11 -o productsapp  main.cpp functions.cpp -lcpprest -lboost_system -lssl -lcrypto```

- std=c++11: Especifica el estándar de C++ a usar, necesario para algunas características de C++ usadas con libcpprest.
      main.cpp: El nombre del source.cpp
- o productsapp: Especifica el nombre del archivo de salida ejecutable. Puedes cambiar my_service por cualquier otro nombre que prefieras para tu programa.
- lcpprest: Vincula el programa con la biblioteca libcpprest.
- lboost_system: Vincula el programa con la biblioteca Boost.System, que es una de las dependencias de libcpprest.
- lssl -lcrypto: Vincula el programa con las bibliotecas OpenSSL, que son necesarias para el soporte de HTTPS y otras características de seguridad.
- functions.cpp es la implementación que se incluye en el main

- *Ejecutarlo* 
    ```./productsapp```

- *Consumirlo*
    - local: ```curl "http://localhost:5000?id=200"```
    
### Testing

- Bajar el paquete de  gtest  
```sudo apt install -y libgtest-dev``` 
    
    Para fedora:
```sudo dnf install googletest-devel```

- Compilar las unitTest

    ```g++ -std=c++11 -o runUnitTest unitTest.cpp functions.cpp -lgtest -lgtest_main -lpthread -lcpprest -lboost_system -lssl -lcrypto```

- Ejecutar las unitTest 
 ```./runUnitTest```

### Jenkis en EC2

1. Crear una EC2 y aisgnar un nombre como: _JenkisServer_
2. Seleccionar la AMI. _Ubuntu Server 22.04 Free tier_
3. Instace type: _t2.midle_ (Con una inferior es posible que el container se detenga)
4. Seleccionar o crear Key pair.
5. Create security group: _Allow SSH anywhere, allow Https, htpp from internet_
6. Configure storage: _default 8 Gb gp2_
7. Advance details: _Select the instance profile_ or create a new one
8. Habilitar el puerto 8080 
    - Seleccionar la instancia
    - Ir a Security
    - Seleccionar el Security groups wizard
    - Seleccionarlo y Click en Inboud rules
    - Edit inbound rules 
    - Add rule
    - Custom TCP - 8080 - Anywhere
    - Custom TCP - 5000 - Anywhere
    - SSH - 22 - Anywhere
    - Save rules
9. Launch Instance
    
-  ### Instalar jenkis

- Connect _EC2 Instance conenct _ o via SSH.
- Install Jenkis 
[Instalación en Ubuntu](https://pkg.origin.jenkins.io/debian-stable/)

sudo wget -O /usr/share/keyrings/jenkins-keyring.asc \
    https://pkg.jenkins.io/debian-stable/jenkins.io-2023.key
  
Then add a Jenkins apt repository entry:
    
  echo deb [signed-by=/usr/share/keyrings/jenkins-keyring.asc] \
    https://pkg.jenkins.io/debian-stable binary/ | sudo tee \
    /etc/apt/sources.list.d/jenkins.list > /dev/null
  
Update your local package index, then finally install Jenkins:

  ```sudo apt-get update```
  ```sudo apt-get install fontconfig openjdk-17-jre```
  ```sudo apt-get install jenkins``` 

- Correr el servicio de Jenkis
 journalctl -u jenkins.service

- Conectarse a jenkis 

 ```https://ec2instance.ip:8080```

 Si hay algún error con https(puede ser SSL de Jenkis), intentar con http

- Ver La clave

```sudo cat /var/lib/jenkins/secrets/initialAdminPassword```

 - Instalar los pluguins recomendados.
 - Instalar el GitHub Plugin para los webhooks
 - test webhook
 - Instalar AWS CLI en el Agente de Jenkins (EC2) para poder correr comandos de aws.
    ```sudo apt-get install awscli```

#### Crear repository en dockerhub

- Crear un repositorio en: https://hub.docker.com/
- guardar el user name, el repostoryName para el Pipeline. : jhonarias91/productsapirepo

#### Agregar credentials dockerhub

Primero, almacenar las credenciales de Docker Hub (o de cualquier otro registro Docker) en Jenkins:

1. Ve al panel de Jenkins.
2. Haz clic en “Manage Jenkins” > “Manage Credentials”.
3. Selecciona el almacen (store) y el dominio adecuados donde deseas guardar tus credenciales.
4. Haz clic en “Add Credentials” en el lado izquierdo.
5. Selecciona el tipo “Username with password”.
6. Ingresa tu nombre de usuario y contraseña de Docker Hub.
7. Asigna un ID a estas credenciales que usarás en tu Jenkinsfile, por ejemplo, docker_hub_credentials_id.

Esta credencial será usada en la stage: "Login Push Docker Image" del Pipeline.


#### Agregar credentials AWS

Primero, almacenar las credenciales de AWS en Jenkins:

1. Instalar el plugin AWS CredentialsVersion 218.v1b_e9466ec5da_ o superior. 
2. Ve al panel de Jenkins.
3. Haz clic en “Manage Jenkins” > “Manage Credentials”.
4. Selecciona el almacen (store) y el dominio adecuados donde deseas guardar tus credenciales.
5. AWS Credentials
6. Asignar un id: "awsCredentials"
7. Para conseguir el Access Key ID y Secret Access Key:
 - Ir a IAM, seleccionar o crear un usuario
 - Al grupo de ese usuario darle permisos de S3.
 - Darle la política de: "AdministratorAccess-AWSElasticBeanstalk" para poder crear Beanstalk
 - Click en el usuario y buscar Security Credentials
 - En Access keys (0), darle en create access key:
 - Seleccionar Third-party service.
 - user_access_key_thirdparty
 - Created access key.
 - Copiar ambos valores en Jenkins.
8. Create.

. Al grupo del usuario agregar AmazonS3FullAccess.

Esta credencial será usada en la stage: "Login Push Docker Image" del Pipeline.

#### Crear bucket en S3

Se debe crear un bucket en S3 para subir el archivo Dockerrun.aws.json 
que es el que va a leer el beanstalk.

 1. En Amazon S3 crear nuevo bucket
 2. Dar cualquier nombre como: "productsapicppbucket"
 3. Create bucket

#### Crear Elastic Beanstalk enviroments

1. Configure environment
    1. Ir a Elastic Beanstalk
    2. Create application
    3. Application name: "productsApi"
    4. Environment name: productsApi-preprod
    5. Seleccionar la región en este caso(us-east-2)
    6. Platform: Docker
    7. Single instance
    8. Next
2. Configure service access
    1. Service role: seleccionar el role del usuario con las credenciales asignadas en Jenkins
    2. EC2 instance profile: Mismo del de las credenciales
    3. Next
3. Set up networking, database, and tags
    1. No VPC.
    2. Next
4. Configure instance traffic and scaling
    1. EC2 security groups: seleccionar los del usuario con las credenciales
    2. Instaces types: t3.micro, t3.small
    3. AMI ID: nos dío (ami-0ccc7b852467bcc7f)    
    4. Next
5. Configure updates, monitoring, and logging
    1. Todo por defecto
    2. Next
6. Review
    1. Submit

### Docker deploy

- Instalar docker
    - sudo apt-get update: Actualiza lista de paquetes disponibles.
    - sudo apt-get install -y apt-transport-https ca-certificates curl software-properties-common: Instala utilidades esenciales para añadir repositorios HTTPS.
    - curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -: Añade clave GPG de Docker.
    - sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable": Añade repositorio de Docker.
    - sudo apt-get update: Actualiza lista de paquetes nuevamente.
    - sudo apt-get install -y docker-ce: Instala Docker Community Edition.
    - sudo systemctl start docker: Inicia el servicio de Docker.
    - sudo systemctl enable docker: Habilita Docker al arranque.
    - sudo usermod -aG docker ${USER}: Añade usuario al grupo Docker.

 ```sudo apt-get update```

```sudo apt-get install -y apt-transport-https ca-certificates curl software-properties-common```

```curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"```

```sudo apt-get update```

```sudo apt-get install -y docker-ce```

```sudo systemctl start docker```

```sudo systemctl enable docker```

```sudo usermod -aG docker ${USER}```


### Preparar ambiente para compilación Cpp en EC2 (dev enviroment)

 - ```sudo apt install -y git g++ libboost-all-dev libssl-dev libcpprest-dev libgtest-dev```

  - Agregar el usuario de jenkis al grupo docker para poderle dar 
  permisos al usuario jenkis ejecutar comandos docker sin sudo.
    
    ```sudo usermod -aG docker jenkins``` 

- Verificamos que el puerto no esté ocupado

  ```lsof -i :5000```
  
  - Jenkis pipeline

  Seleccionar from SCM repository Jenkinsfile 
  
 
### Create Jenkins Pipeline.

- Agregar un nuevo item y seleccionar Pipeline
- asiganr un nombre: webHookDocker
- *Build Triggers*: GitHub hook trigger for GITScm polling
- *Pipeline*: Pipeline Script from SCM.
- SCM: Git 
    - Repository URL: https://github.com/jhonarias91/cppProductsRestApi
- Branch: */master
- Script Path: Jenkinsfile
- Save
- Hacer un push al repo y verificar.


- ### Jenkis Pipeline Local (solo para ssh _deprecated_)

- Install Jenkis

    [Install Guide](https://pkg.jenkins.io/debian-stable)

- Habilitar el servicio Jenkis

```sudo systemctl enable jenkins```

 - Iniciar el servicio de jenkis

```sudo systemctl start jenkins ```

- Para verificar el status

```sudo systemctl status jenkins```

- Instalar el plugin de Github

- Instalar pluguing SSH Pipeline Steps Version 2.0.68.va_d21a_12a_6476  o superior

- Conectarse a la EC2, ir a /opt y dar permisos (si se va a desplegar a /opt)

- Agregar la ip de EC2  known host

- ssh-keyscan -t rsa 3.138.34.44 

- copiar el contenido en un archivo known_host y copiar ese archivo 


 en alguna ruta como /var/lib/jenkins/workspace

darle permisos de lectura a jenkis sobre ese archivo .
en la parte de knownHostsFile del sshCommand agregar esa ruta.


 ```chmod -R 777 .```

- Configurar el Pipeline

En build Trigger seleccionar: 

- [x] GitHub hook trigger for GITScm polling

```Groovy
pipeline {
    agent any
    stages{
        stage("Hook"){
            steps {
                git url: "https://github.com/jhonarias91/cppProductsRestApi"
            }
        }
        stage("Build"){
            steps {            
                sh "g++ -std=c++11 -o productsapp ./src/main.cpp ./src/functions.cpp -lcpprest -lboost_system -lssl -lcrypto"
            }
        }
        stage("Run"){
            steps{
                sh "chmod +x ./productsapp"
                sh "./productsapp"
            }
        }
    }
}

En gihut ir a Settings -> Webhooks y en PayloadUrl poner:
la url de ngrok o la de EC2.

http://3.145.92.103:8080/github-webhook

```
Ejecutar el build manual la primera vez,en caso tal que no se active el webHooks.

- Crea un tunel público usando ngrok

[Link ngrok](https://ngrok.com/)

- Iniciar ngrok
Agregar el token de auth. Ver en la página

```ngrok http http://localhost:8080```


### Logs

- Log beansttalk 
 
 Entrar a la EC2 instance: ```cat /var/log/eb-engine.log```





