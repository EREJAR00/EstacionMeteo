//PRUEBA.

<?php



    $host = "localhost";		         // host = localhost because database hosted on the same server where PHP files are hosted
    $dbname = "id16956433_ematfg";              // Database name
    $username = "id16956433_admin";		// Database username
    $password = "9(|^1V-4(I3_c9&<";	        // Database password


// Establish connection to MySQL database
$conn = new mysqli($host, $username, $password, $dbname);


// Check if connection established successfully
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

else { echo "Connected to mysql database. <br>"; }


// Select values from MySQL database table

$sql = "SELECT Temperatura, Humedad, Presión, Viento, UV, TVOC, eCO2, H2, Etanol, Fecha, Hora FROM PrimeraMedida";  // Update your tablename here


$result = $conn->query($sql);

echo "<center>";



if ($result->num_rows > 0) {


    // output data of each row
    while($row = $result->fetch_assoc()) {
        echo "<strong>Temperatura:</strong> " . $row["Temperatura"]. " &nbsp <strong>Humedad:</strong> " . $row["Humedad"]. " &nbsp <strong>Presión:</strong> " . $row["Presión"]. " &nbsp <strong>Viento:</strong> " . $row["Viento"]. " &nbsp <strong>Fecha:</strong> " . $row["Fecha"]." &nbsp <strong>Hora:</strong>" .$row["Hora"]. "<p>";
    


}
} else {
    echo "0 results";
}

echo "</center>";

$conn->close();



?>
