from flask import Flask, jsonify, json
from flask_cors import CORS
from api.compilation import compilation_controller
from api.visualisation import visualisation_controller
from api.simulation import simulation_controller
from api.exceptions import UserException
from marshmallow.exceptions import ValidationError

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


@app.errorhandler(UserException)
def register_user_exception(error):
  response = jsonify(error.to_dict())
  response.status_code = error.status_code
  return response


@app.errorhandler(ValidationError)
def register_validation_error(error):
  response = dict({'message': json.dumps(error.messages)})
  return response, 422
