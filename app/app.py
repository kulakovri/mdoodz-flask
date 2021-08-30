from flask import Flask
from flask_cors import CORS
from compilation import compilation_controller
from visualisation import visualisation_controller
from simulation import simulation_controller

app = Flask(__name__)
app.config['CORS_HEADERS'] = 'Content-Type'
cors = CORS(app, resources={r"*": {"origins": "*", "methods": "*", "allow_headers": "*"}}, supports_credentials=True)

origins = app.config.get('CORS_ORIGIN_WHITELIST', '*')
resources = {r"*": {"origins": "*", "supports_credentials": True}}

cors.init_app(compilation_controller.blueprint, origins=origins, resources=resources)
cors.init_app(compilation_controller.cleaner_blueprint, origins=origins, resources=resources)
cors.init_app(visualisation_controller.blueprint, origins=origins, resources=resources)
cors.init_app(simulation_controller.blueprint, origins=origins, resources=resources)
app.register_blueprint(compilation_controller.blueprint)
app.register_blueprint(compilation_controller.cleaner_blueprint)
app.register_blueprint(visualisation_controller.blueprint)
app.register_blueprint(simulation_controller.blueprint)

