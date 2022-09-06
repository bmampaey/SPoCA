#!/usr/bin/env python3
import logging
import argparse
import string
import json
import copy
import pandas
import sys
from collections import defaultdict

# HTML template to generate the document and each chart
FILE_TEMPLATE = string.Template('''
<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="utf-8">
	<meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
		<!-- Amcharts 4 library -->
		<script src="https://cdn.amcharts.com/lib/4/core.js"></script>
		<script src="https://cdn.amcharts.com/lib/4/charts.js"></script>
</head>

<body>
	${charts}
	<footer>
		<p>Generated with command<pre><code>${command}</code></pre></p>
	</footer>
</body>
</html>
''')

CHART_TEMPLATE = string.Template('''
	<div id="${chart_name}" style="width: 100%; height: 500px;"></div>
	<script type="text/javascript">
		am4core.createFromConfig(${chart_config}, "${chart_name}");
	</script>
''')


# Structure templates to be converted to JSON for the AmCharts library
CHART_CONFIG = {
	'type': 'XYChart',
	'titles': [
		{
			'text': '',
			'fontSize': 16,
			'fontWeight': 600
		}
	],
	'xAxes': [
		{
			'type': 'DateAxis',
			'id': 'xAxis1',
			'title': {
				'text': '(UTC)'
			},
			'tooltipDateFormat': 'dd MMM yyyy\n\u2007HH:mm:ss',
			'baseInterval': {
				'timeUnit': 'second',
				'count': 1
			},
			'dateFormats': {
				'second': 'HH:mm:ss',
				'minute': 'HH:mm:ss',
				'hour': 'HH:mm:ss',
				'day': 'dd MMM YYYY',
				'month': 'MMM YYYY',
				'year': 'YYYY'
			},
			'periodChangeDateFormats': {
				'second': 'HH:mm:ss',
				'minute': 'HH:mm:ss',
				'hour': 'dd MMM YYYY',
				'day': 'MMM YYYY',
				'month': 'YYYY',
				'year': 'YYYY'
			},
			'renderer': {
				'labels': {
					'location': 0.001
				}
			}
		}
	],
	'yAxes': [],
	'series': [],
	'dateFormatter': {
		'inputDateFormat': 'i',
		'dateFormat': 'dd MMM yyyy\u2007HH:mm:ss',
		'utc': True
	},
	'fontSize': 12,
	'cursor': {
		'behavior': 'zoomX',
		'lineY': {
			'disabled': True
		}
	},
	'scrollbarX': {
		'type': 'Scrollbar',
		'startGrip': {
			'scale': 0.7
		},
		'endGrip': {
			'scale': 0.7
		}
	},
	'scrollbarY': {
		'type': 'Scrollbar',
		'startGrip': {
			'scale': 0.7
		},
		'endGrip': {
			'scale': 0.7
		}
	},
	'legend': {},
	'data': []
}

SERIES_CONFIG = {
	'type': 'LineSeries',
	'name': '',
	'dataFields': {
		'dateX': 'timestamp',
		'valueY': ''
	},
	'tooltipText': '{valueY}',
	'stroke': '',
	'yAxis': ''
}

YAXIS_CONFIG = {
	'type': 'ValueAxis',
	'id': '',
	'title': {
		'text': ''
	},
	'cursorTooltipEnabled': False
}

# Default colors to use for the different series of the charts, but will be the same for each chart
COLORS = ['black','red', 'green', 'blue', '#969600','#0000f0','#78f0f0','#f000b4','#00f05a','#f00000','#f0f078','#0096f0','#007800','#965af0','#780078','#b49600','#0000d2','#5af0d2','#f05a78']


def get_sources_dataframes(sources):
	'''Read the CSV files as specified by the "--source" argument to create the corresponding dataframe'''
	sources_dataframes = dict()
	
	for csv_file, source_name, time_column in sources:
		logging.info('Reading source %s from file %s', source_name, csv_file)
		dataframe = pandas.read_csv(csv_file, index_col = time_column, parse_dates = True)
		
		# Amcharts require the data to be sorted by time, and strongly encourage to use UNIX timestamps for the dates
		dataframe.sort_index(inplace=True)
		dataframe['timestamp'] = pandas.to_numeric(dataframe.index)/1000000
		
		sources_dataframes[source_name] = dataframe
	
	return sources_dataframes


def get_filters(filters):
	'''Create the filters to apply on the dataframe as specified by the "--filter" argument'''
	result = defaultdict(list)
	for series, column, value in filters:
		result[series].append((column, eval(value)))
	return result


def get_charts_configs(charts, sources_dataframes, filters = {}):
	'''Create the structures describing charts as specified by the "--chart" argument to be converted to JSON for the AmCharts library'''
	
	charts_configs = dict()
	charts_yaxes = dict()
	
	for chart_name, series_name, unit, source_name_column_name in charts:
		
		if chart_name not in charts_configs:
			logging.info('Adding series %s to new chart %s', series_name, chart_name)
			chart_config = copy.deepcopy(CHART_CONFIG)
			chart_config['titles'][0]['text'] = chart_name
			charts_configs[chart_name] = chart_config
		else:
			logging.info('Adding series "%s" to existing chart "%s"', series_name, chart_name)
			chart_config = charts_configs[chart_name]
		
		source_name, column_name = source_name_column_name.split(':', maxsplit = 1)
		
		series_config = copy.deepcopy(SERIES_CONFIG)
		series_config['name'] = series_name
		series_config['data'] = get_data(sources_dataframes[source_name], column_name, filters.get('%s:%s' % (chart_name, series_name), []))
		series_config['dataFields']['valueY'] = column_name
		series_config['yAxis'] = unit
		series_config['stroke'] = COLORS[len(chart_config['series'])]
		
		chart_config['series'].append(series_config)
		
		if chart_name not in charts_yaxes:
			charts_yaxes[chart_name] = set()
		
		if unit not in charts_yaxes[chart_name]:
			yaxis_config = copy.deepcopy(YAXIS_CONFIG)
			yaxis_config['id'] = unit
			yaxis_config['title']['text'] = unit
			if chart_config['yAxes']:
				first_yaxis_id = chart_config['yAxes'][0]['id']
				yaxis_config['syncWithAxis'] = first_yaxis_id
			chart_config['yAxes'].append(yaxis_config)
			charts_yaxes[chart_name].add(unit)
	
	return charts_configs


def get_data(dataframe, column, filters):
	'''Select the value of the column in the dataframe after applying the filters, and return it into a format to be converted to JSON'''
	for filter_column, filter_value in filters:
		logging.debug('Applying filter %s == %s on %s rows', filter_column, filter_value, len(dataframe))
		dataframe = dataframe[dataframe[filter_column] == filter_value]
		logging.debug('Remaining %s rows', len(dataframe))
	
	dataframe = dataframe.get(['timestamp', column])
	dataframe = dataframe.dropna()
	return dataframe.to_dict('records')


def write_html_file(output_file, charts_configs):
	'''Write an HTML file using the templates with the chart_configs converted to JSON for the AmCharts library'''
	charts_html = [CHART_TEMPLATE.substitute(chart_name = chart_name, chart_config = json.dumps(chart_config, indent = 2)) for chart_name, chart_config in charts_configs.items()]
	
	with open(output_file, 'wt') as file:
		file.write(FILE_TEMPLATE.substitute(charts = ''.join(charts_html), command = ' '.join(sys.argv)))


# Start point of the script
if __name__ == '__main__':
	
	# Get the arguments
	parser = argparse.ArgumentParser(description = 'Generate HTML files with charts from CSV files')
	parser.add_argument('--verbose', '-v', choices = ['DEBUG', 'INFO', 'ERROR'], default = 'INFO', help = 'Set the logging level (default is INFO)')
	parser.add_argument('--source', '-s', nargs = 3, metavar = ('CSV-FILE', 'SOURCE-NAME', 'TIME-COLUMN'), action = 'append', required = True, help = 'A CSV source file, the name to reference it in the graphs, the header of the time column for the X axes')
	parser.add_argument('--chart', '-c', nargs = 4, metavar = ('CHART-NAME', 'SERIES-NAME', 'UNIT', 'SOURCE-NAME:COLUMN-NAME'), action = 'append', required = True, help = 'The name of a chart, the name of the series, the unit of the data, the name of the source (CSV file) and the column in the CSV file. If specified multiple times with the same chart name, only one chart will be added but will combine the plots')
	parser.add_argument('--filter', '-f', nargs = 3, metavar = ('CHART-NAME:SERIES-NAME', 'COLUMN-NAME', 'VALUE'), action = 'append', default = list(), help = 'Apply an equal filter on the specified series')
	parser.add_argument('--output', '-o', metavar = 'HTML-FILE', required = True, help = 'The path to the output HTML file')
	args = parser.parse_args()
	
	# Setup the logging
	logging.basicConfig(level = getattr(logging, args.verbose), format = '%(asctime)s %(levelname)-8s: %(message)s')
	
	sources_dataframes = get_sources_dataframes(args.source)
	
	filters = get_filters(args.filter or [])
	
	charts_configs = get_charts_configs(args.chart, sources_dataframes, filters)
	
	write_html_file(args.output, charts_configs)
