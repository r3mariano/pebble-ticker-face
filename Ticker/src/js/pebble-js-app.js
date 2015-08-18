var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function parseLocationData(responseText) {
  // responseText contains a JSON object with weather info
  var json = JSON.parse(responseText);

  // Temperature in Kelvin requires adjustment
  var temperature = Math.round(json.main.temp - 273.15);
  console.log("Temperature is " + temperature);

  var units = localStorage.getItem('weather_units');
  // if in F, convert. No guarantees.
  if (units == 1)
    temperature = Math.round(temperature * 9 / 5 + 32);

  // Conditions
  var conditions = json.weather[0].main;

  if (conditions == "Thunderstorm") conditions = "Storm";
  else if (conditions == "Drizzle") conditions = "Drizzle";
  else if (conditions == "Atmosphere" || conditions == "Extreme" || conditions == "Additional") conditions = json.weather[0].description;
  else if (conditions == "Clouds") {
    if (json.weather[0].id == 800) conditions = "Clear";
    else conditions = "Clouds";
  }

  console.log("Conditions are " + conditions);
  
  // Assemble dictionary using our keys
  var dictionary = {
    "KEY_TEMPERATURE": temperature,
    "KEY_CONDITIONS": conditions
  };

  // Send to Pebble
  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log("Weather info sent to Pebble successfully!");
    },
    function(e) {
      console.log("Error sending weather info to Pebble!");
    }
  );
}

function getWeatherByLocationName(q) {
  // Construct URL
  var url = "http://api.openweathermap.org/data/2.5/weather?q=" +
      q;
 
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', parseLocationData);
}

function locationSuccess(pos) {
  // Construct URL
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude;
 
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', parseLocationData);
}
 
function locationError(err) {
  console.log("Error requesting location!");
}
 
function getWeather() {
  var loc = localStorage.getItem('weather_loc');
  if (loc) {
    getWeatherByLocationName(loc);
  } else {
    navigator.geolocation.getCurrentPosition(
      locationSuccess,
      locationError,
      {timeout: 15000, maximumAge: 60000}
    );
  }
}
 
// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");
 
    // Get the initial weather
    getWeather();
  }
);
 
// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getWeather();
  }
);

Pebble.addEventListener('showConfiguration', function(e) {
  // Get config from local storage
  var cfg = localStorage.getItem('cfg');
  console.log('Sending config: ' + cfg);
  // Show config page
  Pebble.openURL('http://r3mariano.com/ticker/config.html#' + encodeURIComponent(cfg));
});

Pebble.addEventListener('webviewclosed',
  function(e) {
    if (!e.response) return;
    var configuration = JSON.parse(decodeURIComponent(e.response));
    localStorage.setItem('cfg', JSON.stringify(configuration));
    localStorage.setItem('weather_units', configuration['weather_units']);
    localStorage.setItem('weather_loc', configuration['weather_loc']);
    console.log('Configuration window returned: ', JSON.stringify(configuration));

    // Assemble dictionary using our keys
    var dictionary = {
      "KEY_SHOW_SECONDS": configuration['show_seconds'],
      "KEY_DATE_FORMAT": configuration['date_format'],
      "KEY_SHOW_AMPM_24H": configuration['show_ampm_24h']
    };

    // Send to Pebble
    Pebble.sendAppMessage(dictionary,
      function(e) {
        console.log("Config info sent to Pebble successfully!");
      },
      function(e) {
        console.log("Error sending config info to Pebble!");
      }
    );

    // Get the weather again.
    getWeather();
  }
);
