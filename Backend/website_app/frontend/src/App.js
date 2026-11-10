import React, { useCallback, useState } from "react";
import {
  BrowserRouter as Router,
  Routes,
  Route,
  Navigate,
} from "react-router-dom";

// Common components
import Header from "./components/common/Header";
import Sidebar from "./components/common/Sidebar";
import Footer from "./components/common/Footer";
import AlertMessage from "./components/common/AlertMessage";

// Auth components
import LoginPage from "./components/auth/LoginPage";
import RegisterPage from "./components/auth/RegisterPage";
import { canModifyData } from "./services/authService";

// Dashboard
import Dashboard from "./components/dashboard/Dashboard";

// Car Type components
import CarTypeList from "./components/car-types/CarTypeList";
import CarTypeDetail from "./components/car-types/CarTypeDetail";
import CarTypeForm from "./components/car-types/CarTypeForm";
import CarTypeStats from "./components/car-types/CarTypeStats";

// ECU components
import ECUList from "./components/ecus/ECUList";
import ECUDetail from "./components/ecus/ECUDetail";
import CompatibleECUList from "./components/ecus/CompatibleECUList";

// Version components
import VersionList from "./components/versions/VersionList";
import VersionDetail from "./components/versions/VersionDetail";
import VersionUpload from "./components/versions/VersionUpload";

// Feature components
import FeatureList from "./components/features/FeatureList";
import FeatureDetail from "./components/features/FeatureDetail";
import FeatureForm from "./components/features/FeatureForm";

// --- NEW DIAGNOSTIC COMPONENTS (FROM REQUESTS FOLDER) ---
import FleetHealthList from "./components/requests/FleetHealthList";
import VehicleHealthHistory from "./components/requests/VehicleHealthHistory";

function ProtectedRoute({ children }) {
  const token = localStorage.getItem("token");

  if (!token) {
    return <Navigate to="/login" replace />;
  }

  return children;
}

function AdminRestrictedPage({ children }) {
  const canModify = canModifyData();

  if (canModify) {
    return children;
  }

  return (
    <div>
      <div className="alert alert-warning d-flex align-items-center" role="alert">
        <div>
          <strong>View-only access.</strong> You do not have admin clearance to
          modify this data. This page is visible, but editing actions are
          disabled.
        </div>
      </div>

      <div
        style={{
          opacity: 0.45,
          pointerEvents: "none",
          userSelect: "none",
          filter: "grayscale(25%)",
        }}
      >
        {children}
      </div>
    </div>
  );
}

function MainLayout({ alert, setAlert, showAlert }) {
  return (
    <div className="app-container d-flex flex-column min-vh-100">
      <Header />

      <div className="container-fluid flex-grow-1">
        <div className="row">
          <Sidebar />

          <main className="col-md-9 ms-sm-auto col-lg-10 px-md-4 py-4">
            {alert.show && (
              <AlertMessage
                type={alert.type}
                message={alert.message}
                onClose={() =>
                  setAlert({ show: false, type: "", message: "" })
                }
              />
            )}

            <Routes>
              {/* Dashboard */}
              <Route path="/" element={<Dashboard />} />

              {/* --- NEW DIAGNOSTIC ROUTES --- */}
              <Route 
                path="/fleet-health" 
                element={<FleetHealthList showAlert={showAlert} />} 
              />
              <Route 
                path="/diagnostics/history" 
                element={<VehicleHealthHistory showAlert={showAlert} />} 
              />

              {/* Car Types */}
              <Route
                path="/car-types"
                element={<CarTypeList showAlert={showAlert} />}
              />

              <Route
                path="/car-types/new"
                element={
                  <AdminRestrictedPage>
                    <CarTypeForm showAlert={showAlert} />
                  </AdminRestrictedPage>
                }
              />

              <Route
                path="/car-types/edit/:name"
                element={
                  <AdminRestrictedPage>
                    <CarTypeForm showAlert={showAlert} />
                  </AdminRestrictedPage>
                }
              />

              <Route path="/car-types/stats" element={<CarTypeStats />} />

              <Route
                path="/car-types/:name"
                element={<CarTypeDetail showAlert={showAlert} />}
              />

              {/* ECUs */}
              <Route
                path="/ecus/:carTypeName"
                element={<ECUList showAlert={showAlert} />}
              />

              <Route
                path="/ecus/:name/:model"
                element={<ECUDetail showAlert={showAlert} />}
              />

              <Route
                path="/ecus/compatible/:carTypeName"
                element={<CompatibleECUList showAlert={showAlert} />}
              />

              {/* Versions */}
              <Route
                path="/versions/ecu/:ecuName/:ecuModel"
                element={<VersionList showAlert={showAlert} />}
              />

              <Route
                path="/versions/ecu/:ecuName/:ecuModel/:versionNumber"
                element={<VersionDetail showAlert={showAlert} />}
              />

              <Route
                path="/versions/upload"
                element={
                  <AdminRestrictedPage>
                    <VersionUpload showAlert={showAlert} />
                  </AdminRestrictedPage>
                }
              />

              <Route
                path="/versions/compatible/:carTypeName"
                element={
                  <VersionList
                    showAlert={showAlert}
                    isCompatibleView={true}
                  />
                }
              />

              {/* Features */}
              <Route
                path="/features/new"
                element={
                  <AdminRestrictedPage>
                    <FeatureForm showAlert={showAlert} />
                  </AdminRestrictedPage>
                }
              />

              <Route
                path="/features"
                element={<FeatureList showAlert={showAlert} />}
              />

              <Route
                path="/features/:featureCode"
                element={<FeatureDetail showAlert={showAlert} />}
              />

              <Route
                path="/features/edit/:featureCode"
                element={
                  <AdminRestrictedPage>
                    <FeatureForm showAlert={showAlert} />
                  </AdminRestrictedPage>
                }
              />

              {/* Fallback inside dashboard */}
              <Route path="*" element={<Navigate to="/" replace />} />
            </Routes>
          </main>
        </div>
      </div>

      <Footer />
    </div>
  );
}

function App() {
  const [alert, setAlert] = useState({ show: false, type: "", message: "" });

  const showAlert = useCallback((type, message, duration = 5000) => {
    setAlert({ show: true, type, message });

    setTimeout(() => {
      setAlert({ show: false, type: "", message: "" });
    }, duration);
  }, []);

  return (
    <Router>
      <Routes>
        {/* Auth pages without Header / Sidebar / Footer */}
        <Route path="/login" element={<LoginPage />} />

        <Route path="/register" element={<RegisterPage />} />

        {/* Main app pages protected by login */}
        <Route
          path="/*"
          element={
            <ProtectedRoute>
              <MainLayout
                alert={alert}
                setAlert={setAlert}
                showAlert={showAlert}
              />
            </ProtectedRoute>
          }
        />
      </Routes>
    </Router>
  );
}

export default App;