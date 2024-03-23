pipeline {
    agent any
    environment {
        // Definimos variables de entorno
        //El nombre debe coincidir con el repositorio, o agregarle el tag con docker
        DOCKER_IMAGE = 'jhonarias91/productsapirepo:v0.0.${BUILD_NUMBER}' 
        DOCKER_CONTAINER_NAME = 'productsapicontainer'
        AWS_REGION = 'us-east-2'
        BEANSTALK_API_NAME = 'productsApi'
        BEANSTALK_API_PREPROD_ENV = 'productsApi-preprod'
        BEANSTALK_API_PROD_ENV = 'productsapiproduction'
        
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
            //Se captura el error, aunque falle, siempre retornar+a SUCES. pero la etapa es marcada como FAILURE
            steps {
                catchError(buildResult: 'SUCCESS', stageResult: 'FAILURE') {
                        sh "g++ -std=c++11 -o runUnitTest ./src/unitTest.cpp ./src/functions.cpp  -lgtest -lgtest_main -lpthread -lcpprest -lboost_system -lssl -lcrypto"
                        sh "./runUnitTest --gtest_output=\'xml:unittestresults.xml\' "
                    }
             }
             post{
                always {
                        junit 'unittestresults.xml'
                    }
                } 
        }
        stage("IntegrationTest")  {
        steps {
                sh "g++ -std=c++11 -o runIntegrationTest ./src/integrationTest.cpp ./src/functions.cpp -lgtest -lgtest_main -lpthread -lcpprest -lboost_system -lssl -lcrypto"
                sh "./runIntegrationTest  --gtest_output=\'xml:integrationtestresults.xml\' "
            }
            post{
                always {
                     junit 'integrationtestresults.xml'
                }
            }  
        }
        stage("Dockerize") {
            steps {
                script {
                    // Se crea la imágen para el Docker
                    sh "docker build -t ${DOCKER_IMAGE} --build-arg BUILD_NUMBER=${BUILD_NUMBER} ."                    
                }
            }
        }
        stage('Push Docker Image') {
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
        stage("Docker Run :Staging") {
            steps {
                script {                    
                    // Se detiene y elimina el contenedor si ya está en ejecución
                    sh "docker stop ${DOCKER_CONTAINER_NAME} || true && docker rm ${DOCKER_CONTAINER_NAME} || true"
                    
                    // Se ejecuta el contenedor
                    sh "docker run -d --name ${DOCKER_CONTAINER_NAME} -p 4300:4300 ${DOCKER_IMAGE}"
                }
            }
        }
        stage("Upload Dockerrun.aws.json") {
            steps {
                // Usar withCredentials para acceder a las credentials defindias en awsCredentials
                withCredentials([[$class: 'AmazonWebServicesCredentialsBinding', credentialsId: 'awsCredentials']]) {
                    script {                    
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
            "ContainerPort": "4300",
            "HostPort": "4300"
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
        stage("Beanstalk - Prod") {
            steps {
                withCredentials([[$class: 'AmazonWebServicesCredentialsBinding', credentialsId: 'awsCredentials']]) {
                    script {                   
                       
                        // Crea una nueva versión de la aplicación en Elastic Beanstalk utilizando el archivo Dockerrun.aws.json de S3
                        sh "aws elasticbeanstalk create-application-version --region ${AWS_REGION} --application-name ${BEANSTALK_API_NAME} --version-label ${BUILD_NUMBER} --source-bundle S3Bucket=\"productsapicppbucket\",S3Key=\"${BUILD_NUMBER}/Dockerrun.aws.json\""

                        // Actualiza el entorno de Elastic Beanstalk para usar la nueva versión de la aplicación
                        sh "aws elasticbeanstalk update-environment --application-name ${BEANSTALK_API_NAME} --region ${AWS_REGION} --environment-name ${BEANSTALK_API_PROD_ENV} --version-label ${BUILD_NUMBER}"
                    }
                }
            }
        }
        stage("Verify Deployment") {
            when {
                expression { false } // Disabled stage
            }
            steps {
                script {                
                    int retries = 10
                    while (true) {
                        try {
                            // Intenta obtener la respuesta del servidor
                            def response = sh(script: "curl --silent --fail 'http://productsapiproduction.eba-ijpjgjya.us-east-2.elasticbeanstalk.com/health'", returnStdout: true).trim()
                            def health = readJSON text: response
                            echo response
                            if (health.version == "${BUILD_NUMBER}") {
                                echo "Verified deployment of version ${BUILD_NUMBER}"
                                break // Sale del bucle si la versión coincide
                            } else {
                                echo "Current version ${health.version} not match with ${BUILD_NUMBER}"
                            }
                        } catch (Exception e) {
                            // Maneja el caso en que curl falla porque el servidor no responde
                            echo "The server did not respond or an error ocurred: ${e.getMessage()}"
                        }

                        if (retries-- == 0) {
                            error "Versión ${BUILD_NUMBER} no found after several tries."
                        }

                        echo "Waiting for retry... remaining attempts: ${retries}"
                        sleep 10 // Espera 10 segundos antes de reintentar
                    }
                }
            }
        }
    }
}
