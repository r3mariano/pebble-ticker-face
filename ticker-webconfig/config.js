// 2015 Ricardo Mariano

$('#config_form').submit(function(event) {
	var f = $('#config_form');
	var show_seconds = f.find('[name=show_seconds]:checked').val();
	var show_ampm_24h = f.find('[name=show_ampm_24h]:checked').val();
	var date_format = f.find('[name=date_format]:checked').val();
	var weather_units = f.find('[name=weather_units]:checked').val();
	var location = $('#location_name').val();

	var outJSON = {
		'show_seconds': parseInt(show_seconds),
		'show_ampm_24h': parseInt(show_ampm_24h),
		'date_format': parseInt(date_format),
		'weather_loc': location,
		'weather_units': parseInt(weather_units)
	}

	document.location.href = 'pebblejs://close#' + encodeURIComponent(JSON.stringify(outJSON));

	event.preventDefault();
});

function selectEl(el) {
	el.prop('checked', true);
	el.parent().parent().children().each(function(idx, e) {
		$(e).removeClass('active');
	})
	el.parent().addClass('active');
}

$(document).ready(function(event) {
	if (window.location.hash) {
		var rawhash = window.location.hash.substring(1);
		var configJSON = JSON.parse(decodeURIComponent(rawhash));
		var f = $('#config_form');
		var el = f.find('[name=show_seconds][value=' + configJSON['show_seconds'] + ']');
		selectEl(el);
		el = f.find('[name=date_format][value=' + configJSON['date_format'] + ']');
		selectEl(el);
		el = f.find('[name=weather_units][value=' + configJSON['weather_units'] + ']');
		selectEl(el);
		if (configJSON['show_ampm_24h']) {
			el = f.find('[name=show_ampm_24h][value=' + configJSON['show_ampm_24h'] + ']');
			selectEl(el);
		}
		$('#location_name').val(configJSON['weather_loc']);
	}
})