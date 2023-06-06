@Library("PSL") _
    
def axisNode = ["GEC-vs2019-cicd", "GEC-xcode12-cicd", "GEC-gcc485-gfx"]

if (currentBuild.rawBuild.getCauses().toString().contains('BranchIndexingCause')) {
  print "INFO: Build skipped due to trigger being Branch Indexing"
  currentBuild.result = 'ABORTED'
  return
}

def tasks = [:]

for(int i=0; i< axisNode.size(); i++) {
    def axisNodeValue = axisNode[i]
    tasks["${axisNodeValue}"] = {
        node(axisNodeValue) {
            def components = env.JOB_NAME.split('/')
            env.COMPONENT = components[1].toLowerCase()
            def branch = "${env.BRANCH_NAME}"
            def WorkDir
            def WorkDirComp
            if(axisNodeValue.contains("GEC-vs")) {
                WorkDir="D:\\Stage\\workspace\\${env.COMPONENT}\\" + env.BRANCH_NAME.replace("/", "\\")
                WorkDirComp = WorkDir + "\\${env.COMPONENT}"
            } else {
                WorkDir="/Volumes/DATA/workspace/${env.COMPONENT}/${env.BRANCH_NAME}"
                WorkDirComp = WorkDir + "/${env.COMPONENT}"
            }
            try {
                ws("${WorkDirComp}"){
                    stage("Sync") {
                        println "Node=${env.NODE_NAME}"
                    
                        checkout([$class: 'GitSCM', branches: scm.branches, doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'SubmoduleOption', disableSubmodules: false, parentCredentials: true, recursiveSubmodules: true, reference: '', trackingSubmodules: false]], submoduleCfg: [], userRemoteConfigs: scm.userRemoteConfigs])
                        if(axisNodeValue.contains("GEC-vs")) {
                            if(branch.contains("release")) {
                                map = readProperties file: "adsk-build-scripts\\adsk-contrib\\release.properties"
                            } else {
                                echo "This is a non-release branch"
                                map = readProperties file: "adsk-build-scripts\\adsk-contrib\\dev.properties"
                            }
                        } else {
                            if(branch.contains("release")) {
                                map = readProperties file: "adsk-build-scripts/adsk-contrib/release.properties"
                            } else {
                                echo "This is a non-release branch"
                                map = readProperties file: "adsk-build-scripts/adsk-contrib/dev.properties"
                            }
                        }
                        properties = map.collect { key, value -> return key+'='+value }
                        
                        def sha = getCommitSha(axisNodeValue,WorkDirComp).trim()
                        properties.add("GITCOMMIT=${sha}")
                    }
                    stage("Build") {
                        println (properties)
                        if (axisNodeValue.contains("GEC-vs")) {
                            bat "git clean -fdx"
                            bat '''
                            FOR /F "tokens=*" %%g IN ('dir /b C:\\Users\\svc_airbuild\\.jenny\\tools\\cmake') do (SET cmakever=%%g)
                            set cmake=C:\\Users\\svc_airbuild\\.jenny\\tools\\cmake\\%cmakever%\\bin\\cmake
                            %cmake% -S . -B_mtlxbuild -G "Visual Studio 16 2019" -A x64 -DCMAKE_INSTALL_PREFIX=%WORKSPACE%\\install_debug -DMATERIALX_BUILD_PYTHON=OFF  -DMATERIALX_BUILD_RENDER=ON -DMATERIALX_BUILD_TESTS=OFF -DCMAKE_DEBUG_POSTFIX=d  -DMATERIALX_BUILD_GEN_OSL=ON -DMATERIALX_BUILD_GEN_MDL=OFF -DMATERIALX_CONTRIB=ON -DMATERIALX_BUILD_VIEWER=OFF -DMATERIALX_BUILD_SHARED_LIBS=ON -DMATERIALX_INSTALL_INCLUDE_PATH=inc -DMATERIALX_INSTALL_LIB_PATH=libs -DMATERIALX_INSTALL_STDLIB_PATH=libraries
                            %cmake% --build _mtlxbuild --config Debug
                            %cmake% --build _mtlxbuild --target install
                            
                            %cmake% -S . -B_mtlxbuild -G "Visual Studio 16 2019" -A x64 -DCMAKE_INSTALL_PREFIX=%WORKSPACE%\\install_release -DMATERIALX_BUILD_PYTHON=OFF  -DMATERIALX_BUILD_RENDER=ON -DMATERIALX_BUILD_TESTS=OFF -DCMAKE_DEBUG_POSTFIX=d  -DMATERIALX_BUILD_GEN_OSL=ON -DMATERIALX_BUILD_GEN_MDL=OFF -DMATERIALX_CONTRIB=ON -DMATERIALX_BUILD_VIEWER=OFF -DMATERIALX_BUILD_SHARED_LIBS=ON -DMATERIALX_INSTALL_INCLUDE_PATH=inc -DMATERIALX_INSTALL_LIB_PATH=libs -DMATERIALX_INSTALL_STDLIB_PATH=libraries
                            %cmake% --build _mtlxbuild --config Release
                            %cmake% --build _mtlxbuild --config Release --target install
                            '''
                        } else if (axisNodeValue.contains("GEC-xcode")){
                            sh "git clean -fdx"
                            sh '''
                            export cmake_version=$(ls /Users/airbuild/.jenny/tools/cmake | xargs -n 1 basename | tail -1)
                            export cmake_cmd=/Users/airbuild/.jenny/tools/cmake/${cmake_version}/bin/cmake
                            ${cmake_cmd} -S . -B_mtlxbuild -G "Xcode" -DCMAKE_INSTALL_PREFIX=$WORKSPACE/install_debug -DMATERIALX_BUILD_PYTHON=OFF  -DMATERIALX_BUILD_RENDER=ON -DMATERIALX_BUILD_TESTS=OFF -DCMAKE_DEBUG_POSTFIX=d  -DMATERIALX_BUILD_GEN_OSL=ON -DMATERIALX_BUILD_GEN_MDL=OFF -DMATERIALX_CONTRIB=ON -DMATERIALX_BUILD_VIEWER=OFF -DMATERIALX_BUILD_SHARED_LIBS=ON -DMATERIALX_INSTALL_INCLUDE_PATH=inc -DMATERIALX_INSTALL_LIB_PATH=libs -DMATERIALX_INSTALL_STDLIB_PATH=libraries '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64'
                            ${cmake_cmd} --build _mtlxbuild --config Debug
                            ${cmake_cmd} --build _mtlxbuild --target install
                            
                            ${cmake_cmd} -S . -B_mtlxbuild -G "Xcode" -DCMAKE_INSTALL_PREFIX=$WORKSPACE/install_release -DMATERIALX_BUILD_PYTHON=OFF  -DMATERIALX_BUILD_RENDER=ON -DMATERIALX_BUILD_TESTS=OFF -DCMAKE_DEBUG_POSTFIX=d  -DMATERIALX_BUILD_GEN_OSL=ON -DMATERIALX_BUILD_GEN_MDL=OFF -DMATERIALX_CONTRIB=ON -DMATERIALX_BUILD_VIEWER=OFF -DMATERIALX_BUILD_SHARED_LIBS=ON -DMATERIALX_INSTALL_INCLUDE_PATH=inc -DMATERIALX_INSTALL_LIB_PATH=libs -DMATERIALX_INSTALL_STDLIB_PATH=libraries '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64'
                            ${cmake_cmd} --build _mtlxbuild --config Release
                            ${cmake_cmd} --build _mtlxbuild --config Release --target install
                            '''
                        } else {
                            sh "git clean -fdx"
                            sh '''
                            cmake -S . -G "Unix Makefiles" -B_build -DCMAKE_BUILD_TYPE=debug -DCMAKE_INSTALL_PREFIX=_build/install_debug -DMATERIALX_BUILD_PYTHON=OFF  -DMATERIALX_BUILD_RENDER=ON -DMATERIALX_BUILD_TESTS=OFF -DCMAKE_DEBUG_POSTFIX=d  -DMATERIALX_BUILD_GEN_OSL=ON -DMATERIALX_BUILD_GEN_MDL=OFF -DMATERIALX_CONTRIB=ON -DMATERIALX_BUILD_VIEWER=OFF -DMATERIALX_BUILD_SHARED_LIBS=ON -DMATERIALX_INSTALL_INCLUDE_PATH=inc -DMATERIALX_INSTALL_LIB_PATH=libs -DMATERIALX_INSTALL_STDLIB_PATH=libraries 
                            cmake --build _build --config debug --target install --parallel 16
                            cmake -S . -G "Unix Makefiles" -B_build -DCMAKE_BUILD_TYPE=release -DCMAKE_INSTALL_PREFIX=_build/install_release -DMATERIALX_BUILD_PYTHON=OFF  -DMATERIALX_BUILD_RENDER=ON -DMATERIALX_BUILD_TESTS=OFF -DMATERIALX_BUILD_GEN_OSL=ON -DMATERIALX_BUILD_GEN_MDL=OFF -DMATERIALX_CONTRIB=ON -DMATERIALX_BUILD_VIEWER=OFF -DMATERIALX_BUILD_SHARED_LIBS=ON -DMATERIALX_INSTALL_INCLUDE_PATH=inc -DMATERIALX_INSTALL_LIB_PATH=libs -DMATERIALX_INSTALL_STDLIB_PATH=libraries
                            cmake --build _build --config release --target install --parallel 16

                            '''
                        }
                    }
					stage("Sign binaries") {
						if(!isUnix()) {
							signWin("${WORKSPACE}\\install_debug")
							signWin("${WORKSPACE}\\install_release")
						}
					}
                    stage("Create Nuget Packages") {
                        withEnv(properties) {
                            if(branch.contains("release")) {
                                properties.add("NUGET_VERSION=${env.Version}")
                            } else {
                                properties.add("NUGET_VERSION=${env.Version}-${env.GITCOMMIT}")
                            }
                        }
                        withEnv(properties) {
                            if(axisNodeValue.contains("GEC-vs")) {
                                bat """
                                nuget pack adsk-build-scripts\\nuget\\win\\adsk_materialx-headers_win.nuspec -Version %NUGET_VERSION% -OutputDirectory %WORKSPACE%\\packages -Prop installdir=%WORKSPACE%\\install_release -Prop materialx=${materialx_version} -Prop win_compiler=${win_compiler}
                                nuget pack adsk-build-scripts\\nuget\\win\\adsk_materialx-lib_win_debug_intel64.nuspec -Version %NUGET_VERSION% -OutputDirectory %WORKSPACE%\\packages -Prop installdir=%WORKSPACE%\\install_debug -Prop materialx=${materialx_version} -Prop win_compiler=${win_compiler}
                                nuget pack adsk-build-scripts\\nuget\\win\\adsk_materialx-lib_win_release_intel64.nuspec -Version %NUGET_VERSION% -OutputDirectory %WORKSPACE%\\packages -Prop installdir=%WORKSPACE%\\install_release -Prop materialx=${materialx_version} -Prop win_compiler=${win_compiler}
                                nuget pack adsk-build-scripts\\nuget\\win\\adsk_materialx-content.nuspec -Version %NUGET_VERSION% -OutputDirectory %WORKSPACE%\\packages -Prop installdir=%WORKSPACE%\\install_release -Prop materialx=${materialx_version} -Prop materialxcontrib=%WORKSPACE%\\source\\MaterialXContrib -Prop win_compiler=${win_compiler}
                                nuget pack adsk-build-scripts\\nuget\\win\\adsk_materialx-sdk_win_intel64.nuspec -Version %NUGET_VERSION% -OutputDirectory %WORKSPACE%\\packages -Prop installdir=%WORKSPACE%\\install_release -Prop materialx=${materialx_version} -Prop materialxcontrib=%WORKSPACE%\\source\\MaterialXContrib -Prop win_compiler=${win_compiler}
                                nuget pack adsk-build-scripts\\nuget\\win\\adsk_materialx_win_debug_intel64.nuspec -Version %NUGET_VERSION% -OutputDirectory %WORKSPACE%\\packages -Prop installdir=%WORKSPACE%\\install_debug -Prop materialx=${materialx_version} -Prop materialxcontrib=%WORKSPACE%\\source\\MaterialXContrib -Prop win_compiler=${win_compiler}
                                nuget pack adsk-build-scripts\\nuget\\win\\adsk_materialx_win_release_intel64.nuspec -Version %NUGET_VERSION% -OutputDirectory %WORKSPACE%\\packages -Prop installdir=%WORKSPACE%\\install_release -Prop materialx=${materialx_version} -Prop materialxcontrib=%WORKSPACE%\\source\\MaterialXContrib -Prop win_compiler=${win_compiler}
                                """
                                zip zipFile: "${env.WORKSPACE}/packages/adsk_materialx-sdk_win_intel64.${env.NUGET_VERSION}.zip", dir: "${env.WORKSPACE}/install_release", glob: '**/*'
                            } else if (axisNodeValue.contains("GEC-xcode")){
                                sh """
                                nuget pack adsk-build-scripts/nuget/osx/adsk_materialx-headers_osx.nuspec -Version $NUGET_VERSION -OutputDirectory $WORKSPACE/packages -Prop installdir=$WORKSPACE/install_release -Prop materialx=${materialx_version} -Prop osx_compiler=${osx_compiler} -Prop osx_target=${osx_target}
                                nuget pack adsk-build-scripts/nuget/osx/adsk_materialx_osx_debug_ubx64arm64.nuspec -Version $NUGET_VERSION -OutputDirectory $WORKSPACE/packages -Prop installdir=$WORKSPACE/install_debug -Prop materialx=${materialx_version} -Prop osx_compiler=${osx_compiler} -Prop osx_target=${osx_target}
                                nuget pack adsk-build-scripts/nuget/osx/adsk_materialx_osx_release_ubx64arm64.nuspec -Version $NUGET_VERSION -OutputDirectory $WORKSPACE/packages -Prop installdir=$WORKSPACE/install_release -Prop materialx=${materialx_version} -Prop osx_compiler=${osx_compiler} -Prop osx_target=${osx_target}
                                nuget pack adsk-build-scripts/nuget/osx/adsk_materialx-content.nuspec -Version $NUGET_VERSION -OutputDirectory $WORKSPACE/packages -Prop installdir=$WORKSPACE/install_release -Prop materialxcontrib=$WORKSPACE/source/MaterialXContrib -Prop materialx=${materialx_version} -Prop osx_compiler=${osx_compiler} -Prop osx_target=${osx_target}
                                nuget pack adsk-build-scripts/nuget/osx/adsk_materialx-sdk_osx_ubx64arm64.nuspec -Version $NUGET_VERSION -OutputDirectory $WORKSPACE/packages -Prop installdir=$WORKSPACE/install_release -Prop materialxcontrib=$WORKSPACE/source/MaterialXContrib -Prop materialx=${materialx_version} -Prop osx_compiler=${osx_compiler} -Prop osx_target=${osx_target}
                                """
                                zip zipFile: "${env.WORKSPACE}/packages/adsk_materialx-sdk_osx_ubx64arm64.${env.NUGET_VERSION}.zip", dir: "${env.WORKSPACE}/install_release", glob: '**/*'
                            } else {
                                sh """
                                nuget pack adsk-build-scripts/nuget/linux/adsk_materialx-headers_linux.nuspec -Version $NUGET_VERSION -OutputDirectory $WORKSPACE/packages -Prop installdir=$WORKSPACE/_build/install_release -Prop materialx=${materialx_version} -Prop linux_compiler=${linux_compiler} -Prop linux_target=${linux_target}
                                nuget pack adsk-build-scripts/nuget/linux/adsk_materialx_linux_debug_intel64.nuspec -Version $NUGET_VERSION -OutputDirectory $WORKSPACE/packages -Prop installdir=$WORKSPACE/_build/install_debug -Prop materialx=${materialx_version} -Prop linux_compiler=${linux_compiler} -Prop linux_target=${linux_target}
                                nuget pack adsk-build-scripts/nuget/linux/adsk_materialx_linux_release_intel64.nuspec -Version $NUGET_VERSION -OutputDirectory $WORKSPACE/packages -Prop installdir=$WORKSPACE/_build/install_release -Prop materialx=${materialx_version} -Prop linux_compiler=${linux_compiler} -Prop linux_target=${linux_target}
                                nuget pack adsk-build-scripts/nuget/linux/adsk_materialx-content.nuspec -Version $NUGET_VERSION -OutputDirectory $WORKSPACE/packages -Prop installdir=$WORKSPACE/_build/install_release -Prop materialxcontrib=$WORKSPACE/source/MaterialXContrib -Prop materialx=${materialx_version} -Prop linux_compiler=${linux_compiler} -Prop linux_target=${linux_target}
                                nuget pack adsk-build-scripts/nuget/linux/adsk_materialx-sdk_linux_intel64.nuspec -Version $NUGET_VERSION -OutputDirectory $WORKSPACE/packages -Prop installdir=$WORKSPACE/_build/install_release -Prop materialxcontrib=$WORKSPACE/source/MaterialXContrib -Prop materialx=${materialx_version} -Prop linux_compiler=${linux_compiler} -Prop linux_target=${linux_target}
                                """
                                zip zipFile: "${env.WORKSPACE}/packages/adsk_materialx-sdk_linux_intel64.${env.NUGET_VERSION}.zip", dir: "${env.WORKSPACE}/_build/install_release", glob: '**/*'
                            }
                        }
                    }
                    stage("Upload Artifactory") {
                        withEnv(properties) {
                            uploadArtifactory()
                        }
                    }
                }
            } catch (caughtError) {
                println ("Error: " + caughtError)
                currentBuild.result="FAILURE"
            }
            finally {
                // Clear workspace directory
                dir("${WorkDirComp}") {
                    deleteDir()
                }
            }
        }
    }
}

stage("Build") {
    parallel tasks
}
stage ("Update Version") {
    node("GEC-vs2017") {
        if(!currentBuild.currentResult.contains("FAILURE")) {
            if ("${env.BRANCH_NAME}".contains("release")){
                checkout scm
                withEnv(["file=adsk-build-scripts\\adsk-contrib\\release.properties"]) {
                    withEnv(["branch=${env.BRANCH_NAME}"]) {
                    bat '''
                        git checkout %branch%
                        git pull
                        pushd adsk-build-scripts
                        python versioning.py -u -b release
                        popd
                        git commit %file% -m "Update build version"
                        git push origin %branch%
                    '''
                    }
                }
            }
        }
    }
}

def getCommitSha(axisNodeValue,WorkDirComp){
    if(axisNodeValue.contains("GEC-vs")) {
        def sha = bat(
              returnStdout: true,
              script: """
                    @echo off
                    git rev-parse --short HEAD
                """
            ).trim()
        return sha
    } else {
        return sh(returnStdout: true, script: 'git rev-parse --short HEAD')
    }
}


def uploadArtifactory() {
    echo "INFO: Upload to Artifactory (Nuget)"
    withCredentials([usernamePassword(credentialsId: "832c4de4-8264-4b99-856e-e1d2b5dc2ffc", passwordVariable: 'PASSWORD', usernameVariable: 'USERNAME')]) {
    def server = Artifactory.newServer url: 'https://art-bobcat.autodesk.com/artifactory/', username: "${USERNAME}", password: "${PASSWORD}"
        if ("${env.BRANCH_NAME}".contains("release")){
            def uploadSpec = """{
                "files": [{
                    "pattern": "${env.WORKSPACE}/packages/*.nupkg",
                    "target": "oss-stg-nuget/materialx/${env.NUGET_VERSION}/",
                    "recursive": "false",
                    "props":"git.branch=${env.BRANCH_NAME};git.hash=${env.GITCOMMIT}"
                }]
            }"""
            server.upload(uploadSpec)
        }
        else {    
            def uploadSpec = """{
                "files": [{
                    "pattern": "${env.WORKSPACE}/packages/*.nupkg",
                    "target": "team-gfx-nuget/materialx/${env.NUGET_VERSION}/",
                    "recursive": "false",
                    "props":"git.branch=${env.BRANCH_NAME};git.hash=${env.GITCOMMIT}"
                }]
            }"""
            server.upload(uploadSpec)
        }
    }
    echo "INFO: Upload to Artifactory (Zip)"
    withCredentials([usernamePassword(credentialsId: "832c4de4-8264-4b99-856e-e1d2b5dc2ffc", passwordVariable: 'PASSWORD', usernameVariable: 'USERNAME')]) {
    server = Artifactory.newServer url: 'https://art-bobcat.autodesk.com/artifactory/', username: "${USERNAME}", password: "${PASSWORD}"
        if ("${env.BRANCH_NAME}".contains("release")){
            def uploadSpec = """{
                "files": [{
                    "pattern": "${env.WORKSPACE}/packages/*.zip",
                    "target": "oss-stg-generic/materialx/${env.NUGET_VERSION}/",
                    "recursive": "false",
                    "props":"git.branch=${env.BRANCH_NAME};git.hash=${env.GITCOMMIT}"
                }]
            }"""
            server.upload(uploadSpec)
        }
        else {
            def uploadSpec = """{
                "files": [{
                    "pattern": "${env.WORKSPACE}/packages/*.zip",
                    "target": "team-gfx-generic/materialx/${env.NUGET_VERSION}/",
                    "recursive": "false",
                    "props":"git.branch=${env.BRANCH_NAME};git.hash=${env.GITCOMMIT}"
                }]
            }"""
            server.upload(uploadSpec)
        }
    }   
}
def signWin(String source) {
	withCredentials([usernamePassword(credentialsId: "GEC_garasign", passwordVariable: 'Secret', usernameVariable: 'ID')]) {
		withCredentials([usernamePassword(credentialsId: "3fefba6a-8c13-4321-b5c9-d7d8b92493e8", passwordVariable: 'PASSWORD', usernameVariable: 'USERNAME')]) {
			dir("${WORKSPACE}") {
				bat """
				jfrog rt dl team-engops-stark-generic/garasign/releases/sign.py --user=%USERNAME% --password=%PASSWORD% --url="https://art-bobcat.autodesk.com/artifactory/" --flat=true
				python3 sign.py --config %ID% %Secret% --ttl 6000 --sign --suppressWarning ${source}
				"""
			}
		}
	}
}