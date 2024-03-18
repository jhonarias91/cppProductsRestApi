pipeline {
    agent any
    environment {
        // Definimos variables de entorno
        //El nombre debe coincidir con el repositorio, o agregarle el tag con docker
        DOCKER_IMAGE = 'jhonarias91/productsapirepo:v0.0.${BUILD_NUMBER}' 
        DOCKER_CONTAINER_NAME = 'productsapicontainer'
    }
    stages {
        stage("Checkout") {
            steps {
                // CLonación del proyecto
                git url: "https://github.com/jhonarias91/cppProductsRestApi"
            }
        }
        stage("Build") {
            steps {
                // Build local
                sh "g++ -std=c++11 -o productsapp ./src/main.cpp ./src/functions.cpp -lcpprest -lboost_system -lssl -lcrypto"
            }
        }  
        stage("UnitTest")  {
            steps {
                    sh "g++ -std=c++11 -o runUnitTest ./src/unitTest.cpp ./src/functions.cpp -lgtest -lgtest_main -lpthread -lcpprest -lboost_system -lssl -lcrypto"
                    sh "./runUnitTest"
                }
            }        
        stage("Dockerize") {
            steps {
                script {
                    // Se crea la imágen para el Docker
                    sh "docker build -t ${DOCKER_IMAGE} ."                    
                }
            }
        }
        stage('Login Push Docker Image') {
            steps {
                script {
                    // Usar credenciales de Jenkins para login en Docker Hub
                    withCredentials([usernamePassword(credentialsId: 'docker_hub_credentials_id', passwordVariable: 'DOCKER_HUB_PASSWORD', usernameVariable: 'DOCKER_HUB_USER')]) {
                        sh "echo $DOCKER_HUB_PASSWORD | docker login --username $DOCKER_HUB_USER --password-stdin"
                    }
                    // Opcional: Publicar en un registro de Docker
                    sh "docker push ${DOCKER_IMAGE}"
                }
            }
        }
        stage("Docker Run :Preprod") {
            steps {
                script {                    
                    // Se detiene y elimina el contenedor si ya está en ejecución
                    sh "docker stop ${DOCKER_CONTAINER_NAME} || true && docker rm ${DOCKER_CONTAINER_NAME} || true"
                    
                    // Se ejecuta el contenedor
                    sh "docker run -d --name ${DOCKER_CONTAINER_NAME} -p 5000:5000 ${DOCKER_IMAGE}"
                }
            }
        }
        stage("Prepare and Upload Dockerrun.aws.json") {
            steps {
                script {
                    // Usar withCredentials para acceder a las credentials defindias en awsCredentials
                    withCredentials([[$class: 'AmazonWebServicesCredentialsBinding', credentialsId: 'awsCredentials']]) {
                        // Comandos AWS CLI
                        // Se crea el archivo Dockerrun.aws.json para que beanstalk lo lea
                        sh """
                        cat > Dockerrun.aws.json <<EOL
                        {
                            "AWSEBDockerrunVersion": "1",
                            "Image": {
                                "Name": "${DOCKER_IMAGE}",
                                "Update": "true"
                            },
                            "Ports": [
                                {
                                    "ContainerPort": "5000"
                                }
                            ]
                        }
                        EOL
                        """
                        // Sube el archivo a S3
                        sh "aws s3 cp Dockerrun.aws.json s3://productsapicppbucket/${BUILD_NUMBER}/Dockerrun.aws.json"
                    }
                }
            }
        }

    }
}
