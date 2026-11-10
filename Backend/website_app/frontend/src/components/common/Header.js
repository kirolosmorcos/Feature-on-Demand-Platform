import React from "react";
import { Link, useNavigate } from "react-router-dom";
import { logoutUser, getCurrentUser } from "../../services/authService";

function Header() {
  const navigate = useNavigate();
  const user = getCurrentUser();

  const handleLogout = () => {
    logoutUser();
    navigate("/login");
  };

  const displayName = user?.name
    ? user.name.charAt(0).toUpperCase() + user.name.slice(1)
    : user?.email || "";

  return (
    <header>
      <nav className="navbar navbar-expand-lg navbar-dark bg-dark">
        <div className="container-fluid">
          <Link className="navbar-brand" to="/">
            Automotive Firmware Management
          </Link>

          <button
            className="navbar-toggler"
            type="button"
            data-bs-toggle="collapse"
            data-bs-target="#navbarNav"
            aria-controls="navbarNav"
            aria-expanded="false"
            aria-label="Toggle navigation"
          >
            <span className="navbar-toggler-icon"></span>
          </button>

          <div className="collapse navbar-collapse" id="navbarNav">
            <ul className="navbar-nav ms-auto align-items-lg-center">
              <li className="nav-item">
                <Link className="nav-link" to="/">
                  Dashboard
                </Link>
              </li>

              <li className="nav-item dropdown">
                <a
                  className="nav-link dropdown-toggle"
                  href="#"
                  id="navbarDropdownCars"
                  role="button"
                  data-bs-toggle="dropdown"
                  aria-expanded="false"
                  onClick={(e) => e.preventDefault()}
                >
                  Car Types
                </a>

                <ul
                  className="dropdown-menu"
                  aria-labelledby="navbarDropdownCars"
                >
                  <li>
                    <Link className="dropdown-item" to="/car-types">
                      View All
                    </Link>
                  </li>

                  <li>
                    <Link className="dropdown-item" to="/car-types/new">
                      Add New
                    </Link>
                  </li>

                  <li>
                    <Link className="dropdown-item" to="/car-types/stats">
                      Statistics
                    </Link>
                  </li>
                </ul>
              </li>

              <li className="nav-item">
                <Link className="nav-link" to="/versions/upload">
                  Upload Firmware
                </Link>
              </li>
            </ul>

            <div className="d-flex align-items-center ms-lg-4 mt-2 mt-lg-0">
              {user && (
                <span
                  className="navbar-text me-3"
                  style={{
                    color: "rgba(255,255,255,0.85)",
                    fontWeight: "500",
                    whiteSpace: "nowrap",
                  }}
                >
                  Hi, {displayName}
                </span>
              )}

              <button
                type="button"
                className="nav-link bg-transparent border-0 p-0"
                onClick={handleLogout}
                style={{
                  cursor: "pointer",
                  color: "rgba(255,255,255,0.55)",
                }}
              >
                Logout
              </button>
            </div>
          </div>
        </div>
      </nav>
    </header>
  );
}

export default Header;