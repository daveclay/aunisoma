import org.jetbrains.kotlin.gradle.tasks.KotlinCompile

plugins {
    kotlin("jvm") version "1.7.0"
    application
}

group = "com.daveclay.art"
version = "1.0-SNAPSHOT"

repositories {
    mavenCentral()
    maven(url = "https://clojars.org/repo/")
}


dependencies {
    implementation("net.beadsproject:beads:3.2")
    implementation("org.clojars.automata:tritonus-aos:1.0.0")
    // Processing 4 - from the Download application contents
    // find  /Applications/Processing.app/Contents/Java -name "*.jar"                                                                                                                      Sat 158:17:17
    implementation(files("/Applications/Processing.app/Contents/Java/core.jar"))
    testImplementation(kotlin("test"))
    testImplementation(platform("org.junit:junit-bom:5.9.2"))
    testImplementation("org.junit.jupiter:junit-jupiter")
}

tasks.test {
    useJUnitPlatform()
}

tasks.withType<KotlinCompile> {
    kotlinOptions.jvmTarget = "1.8"
}

application {
    mainClass.set("MainKt")
}