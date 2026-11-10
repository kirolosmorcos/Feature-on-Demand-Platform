import React, { useState } from "react";
import { useNavigate, Link } from "react-router-dom";
import { registerUser } from "../../services/authService";

function RegisterPage() {
  const navigate = useNavigate();

  const [formData, setFormData] = useState({
    name: "",
    email: "",
    password: "",
  });

  const [touched, setTouched] = useState({
    name: false,
    email: false,
    password: false,
  });

  const [showPassword, setShowPassword] = useState(false);
  const [submitted, setSubmitted] = useState(false);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState("");
  const [success, setSuccess] = useState("");

  const name = formData.name.trim();
  const email = formData.email.trim();
  const password = formData.password;

  const isNameValid = name.length > 0;
  const isEmailValid = /^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(email);
  const isPasswordValid = password.length >= 6;

  const isFormValid = isNameValid && isEmailValid && isPasswordValid;

  const showNameError = (touched.name || submitted) && !isNameValid;
  const showEmailError = (touched.email || submitted) && !isEmailValid;
  const showPasswordError = (touched.password || submitted) && !isPasswordValid;

  const handleChange = (e) => {
    setFormData({
      ...formData,
      [e.target.name]: e.target.value,
    });
    setError("");
    setSuccess("");
  };

  const handleBlur = (e) => {
    setTouched({
      ...touched,
      [e.target.name]: true,
    });
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    setSubmitted(true);
    setSuccess("");

    if (!isFormValid) {
      setError("Please complete the form correctly before registering.");
      return;
    }

    try {
      setLoading(true);
      setError("");
      await registerUser({ name, email, password });
      setSuccess("Registration successful. Redirecting to login...");
      setTimeout(() => {
        navigate("/login");
      }, 1200);
    } catch (err) {
      setError(err.message || "Registration failed");
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="bg-light min-vh-100">
      {/* Empty Header */}
      <nav className="navbar navbar-dark bg-dark">
        <div className="container-fluid">
          <span className="navbar-brand mb-0 h1">
            Automotive Firmware Management
          </span>
        </div>
      </nav>

      <div className="container d-flex align-items-center justify-content-center" style={{ minHeight: "calc(100vh - 56px)" }}>
        <div className="card shadow-sm" style={{ width: "100%", maxWidth: "450px" }}>
          <div className="card-header text-center py-3 bg-white">
            <h2 className="h4 mb-0">
              <i className="bi bi-cpu text-primary me-2"></i>
              Register for FodOTA
            </h2>
          </div>
          <div className="card-body p-4">
            <p className="text-muted text-center mb-4">Create a FoD OTA management account.</p>

            {error && <div className="alert alert-danger py-2">{error}</div>}
            {success && <div className="alert alert-success py-2">{success}</div>}

            <form onSubmit={handleSubmit} noValidate>
              
              <div className="mb-3">
                <label htmlFor="name" className="form-label">Name</label>
                <div className="input-group">
                  <span className="input-group-text"><i className="bi bi-person"></i></span>
                  <input
                    id="name"
                    type="text"
                    name="name"
                    className={`form-control ${showNameError ? 'is-invalid' : ''}`}
                    placeholder="John Emile"
                    value={formData.name}
                    onChange={handleChange}
                    onBlur={handleBlur}
                    disabled={loading || Boolean(success)}
                  />
                  {showNameError && <div className="invalid-feedback">Name is required.</div>}
                </div>
              </div>

              <div className="mb-3">
                <label htmlFor="email" className="form-label">Email</label>
                <div className="input-group">
                  <span className="input-group-text"><i className="bi bi-envelope"></i></span>
                  <input
                    id="email"
                    type="email"
                    name="email"
                    className={`form-control ${showEmailError ? 'is-invalid' : ''}`}
                    placeholder="user@example.com"
                    value={formData.email}
                    onChange={handleChange}
                    onBlur={handleBlur}
                    disabled={loading || Boolean(success)}
                  />
                  {showEmailError && <div className="invalid-feedback">Please enter a valid email.</div>}
                </div>
              </div>

              <div className="mb-3">
                <label htmlFor="password" className="form-label">Password</label>
                <div className="input-group">
                  <span className="input-group-text"><i className="bi bi-lock"></i></span>
                  <input
                    id="password"
                    type={showPassword ? "text" : "password"}
                    name="password"
                    className={`form-control ${showPasswordError ? 'is-invalid' : ''}`}
                    placeholder="Create a password"
                    value={formData.password}
                    onChange={handleChange}
                    onBlur={handleBlur}
                    disabled={loading || Boolean(success)}
                  />
                  <button 
                    className="btn btn-outline-secondary" 
                    type="button"
                    onClick={() => setShowPassword(!showPassword)}
                  >
                    <i className={showPassword ? "bi bi-eye-slash" : "bi bi-eye"}></i>
                  </button>
                  {/* Warning placed safely inside input-group to flow naturally below */}
                  {showPasswordError && <div className="invalid-feedback">Password must be at least 6 characters.</div>}
                </div>
                {/* Regular helper text placed below to prevent the overlapping image issue */}
                {!showPasswordError && <div className="form-text">Minimum 6 characters.</div>}
              </div>

              <button
                type="submit"
                disabled={!isFormValid || loading || Boolean(success)}
                className="btn btn-primary w-100 mt-3"
              >
                {loading ? "Creating account..." : "Register"}
              </button>
            </form>

            <div className="text-center mt-4 pt-2 border-top">
              <small className="text-muted">
                Already have an account? <Link to="/login" className="text-decoration-none fw-bold">Login</Link>
              </small>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

export default RegisterPage;