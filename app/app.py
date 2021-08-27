from flask import Flask
from flask_cors import CORS
import mdoodz, visualise

app = Flask(__name__)
app.config['CORS_HEADERS'] = 'Content-Type'
cors = CORS(app, resources={r"/*": {"origins": "*", "methods": "*", "allow_headers": "*"}}, supports_credentials=True)

origins = app.config.get('CORS_ORIGIN_WHITELIST', '*')

cors.init_app(mdoodz.views.compiler, origins=origins)
cors.init_app(mdoodz.views.cleaner, origins=origins)
cors.init_app(mdoodz.views.simulation_runner, origins=origins)
cors.init_app(visualise.views.visualizer, origins=origins)
app.register_blueprint(mdoodz.views.compiler)
app.register_blueprint(mdoodz.views.cleaner)
app.register_blueprint(mdoodz.views.simulation_runner)
app.register_blueprint(visualise.views.visualizer)

